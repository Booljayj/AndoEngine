#include "Rendering/RenderTarget.h"
#include "Engine/GLM.h"
#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Views/View.h"
#include "Rendering/Vulkan/FrameResources.h"
#include "Rendering/Vulkan/GraphicsQueue.h"
#include "Rendering/Vulkan/RenderPasses.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	bool RenderTarget::Render(RenderPasses const& passes, entt::registry const& registry, ResourcesCollection& previous_resources) {
		//@todo Each view should define which entity registry to use when rendering, probably by referencing some kind of "World" object that contains a registry of coexistant entities.
		auto const renderables = registry.view<MeshRenderer const>();

		t_vector<ViewRenderingParameters> view_parameters;
		{
			glm::u32vec2 const extent = GetExtent();

			auto const views = ts_views.LockInclusive();

			view_parameters.resize(views->size());
			//@todo We can split this up so that the parameter collection of all views is done in parallel, including entity collection.
			for (size_t view_index = 0; view_index < views->size(); ++view_index) {
				auto& view = views[view_index];
				auto& params = view_parameters[view_index];

				params.rect = view.rect_calculator ? view.rect_calculator->Calculate(extent) : ViewRect{ glm::zero<glm::i32vec2>(), extent };
				params.camera = view.camera_calculator ? view.camera_calculator->Calculate(registry, params.rect.extent) : ViewCamera{};

				params.frustum = glm::identity<glm::mat4>();
				params.num_threads = view.num_threads;

				params.entities.reserve(512);
				for (entt::entity const entity : renderables) {
					//@todo Perform frustum culling here, as well as any other basic visibility filtering
					params.entities.push_back(entity);
				}
			}
		}

		size_t const num_total_threads = std::accumulate(view_parameters.begin(), view_parameters.end(), 0ull, [](size_t total, ViewRenderingParameters const& params) { return total + params.num_threads; });

		if (FrameResources* frame = PrepareFrame(view_parameters, previous_resources)) {
			//The inheritance info that is shared by all secondary command buffers
			CommandInheritance const inheritance{
				.renderPass = passes.surface,
				.framebuffer = GetFramebuffer(frame->current_image_index),
			};

			//Start the worker threads that will begin adding commands to the secondary command buffers. We want this to happen in parallel with as much of the main thread work as possible.
			std::vector<std::jthread> workers;
			workers.reserve(num_total_threads);

			//@todo This should ideally be done with some acceleration structure that contains a mapping between the pipelines and all of
			//      the geometry that should be drawn with that pipeline, to avoid binding the same pipeline more than once and to strip
			//      out culled geometry.
			for (size_t view_index = 0; view_index < view_parameters.size(); ++view_index) {
				const auto draw_view_partial = [&renderables, &view = frame->views[view_index], &view_params = view_parameters[view_index], &inheritance](size_t thread_index) {
					ThreadBuffer buffer{ 20'000 };

					ThreadResources& thread_resources = view.threads[thread_index];

					size_t const objects_per_thread = view_params.entities.size() / view_params.num_threads;
					size_t const extra_objects = view_params.entities.size() % view_params.num_threads;

					size_t const start = thread_index * objects_per_thread + (thread_index < extra_objects ? thread_index : extra_objects);
					size_t const end = start + objects_per_thread + static_cast<size_t>(thread_index < extra_objects);

					GraphicsCommandWriter const commands{ view.thread_command_buffers[thread_index], inheritance };

					VkViewport const viewport{
						.x = static_cast<float>(view_params.rect.offset.x),
						.y = static_cast<float>(view_params.rect.offset.y),
						.width = static_cast<float>(view_params.rect.extent.x),
						.height = static_cast<float>(view_params.rect.extent.y),
						.minDepth = 0.0f,
						.maxDepth = 1.0f,
					};
					VkRect2D const scissor{
						.offset = { view_params.rect.offset.x, view_params.rect.offset.y },
						.extent = { view_params.rect.extent.x, view_params.rect.extent.y },
					};

					commands.SetViewports(0, MakeSpan(viewport));
					commands.SetScissors(0, MakeSpan(scissor));

					for (size_t index = start; index < end; ++index) {
						entt::entity const entity = view_params.entities[index];
						auto const& renderer = renderables.get<MeshRenderer const>(entity);

						Material const* material = renderer.material.get();
						StaticMesh const* mesh = renderer.mesh.get();

						if (material && material->objects && mesh && mesh->objects) {
							thread_resources.resources += material->objects;
							thread_resources.resources += mesh->objects;

							//Write this object's data into the object buffer, and keep track of the index for it
							uint32_t const object_buffer_index = view.uniforms.object.Write(
								ObjectUniforms{
									.modelViewProjection = glm::identity<glm::mat4>(),
								},
								index
							);

							//Bind the pipeline that will be used for rendering
							commands.BindGraphicsPipeline(material->objects->pipeline);

							//Bind the descriptor sets to use for this draw command
							EnumArray<VkDescriptorSet, EGraphicsLayouts> sets;
							sets[EGraphicsLayouts::Global] = view.uniforms.global;
							sets[EGraphicsLayouts::Object] = view.uniforms.object;
							//sets[EGraphicsLayouts::Material] = material->gpuResources->set;

							enum class EDynamicOffsets {
								Object,
								MAX,
							};
							EnumArray<uint32_t, EDynamicOffsets> offsets;
							offsets[EDynamicOffsets::Object] = object_buffer_index;
							//offsets[EDynamicOffsets::Material] = ???

							commands.BindGraphicsDescriptorSets(material->objects->layouts.pipeline, 0, MakeSpan(sets), MakeSpan(offsets));

							//Bind the vetex and index buffers for the mesh
							VkBuffer const mesh_buffer = mesh->objects->buffer;
							commands.BindVertexBuffers(0, MakeSpan(mesh_buffer), MakeSpan(mesh->objects->offset.vertex));
							commands.BindIndexBuffer(mesh_buffer, mesh->objects->offset.index, mesh->objects->indexType);

							//Submit the command to draw using the vertex and index buffers
							constexpr uint32_t instance_count = 1;
							commands.DrawIndexed(0, mesh->objects->size.indices, 0, instance_count);
						}
					}
				};

				for (size_t thread_index = 0; thread_index < view_parameters[view_index].num_threads; ++thread_index) {
					workers.emplace_back(draw_view_partial, thread_index);
				}
			}

			//Write global uniform values that can be accessed by shaders
			for (size_t view_index = 0; view_index < view_parameters.size(); ++view_index) {
				ViewRenderingParameters const& view_params = view_parameters[view_index];
				ViewResources const& view = frame->views[view_index];

				glm::mat4 const view_matrix = view_params.camera.transform;
				glm::mat4 projection_matrix = glm::perspective(glm::radians(view_params.camera.fov), view_params.camera.aspect, view_params.camera.clip.near, view_params.camera.clip.far);
				projection_matrix[1][1] *= -1.0f; //flip this coordinate to account for differences in the Y-axis between OpenGL and Vulkan.
				glm::mat4 const view_projection_matrix = projection_matrix * view_matrix;

				view.uniforms.global.Write(
					GlobalUniforms{
						.viewProjection = view_projection_matrix,
						.viewProjectionInverse = glm::inverse(view_projection_matrix),
						.time = 0,
					}
				);
			}

			//Wait for the workers to finish, and then add their commands to the primary command buffer. We do this by clearing to make sure workers are not used beyond this point.
			workers.clear();

			for (size_t view_index = 0; view_index < view_parameters.size(); ++view_index) {
				GraphicsCommandWriter const commands{ frame->view_command_buffers[view_index] };

				//Create a scope for all commands that belong to the surface render pass
				SurfaceRenderPass::ScopedRecord const scopedRecord{ commands, passes.surface, GetFramebuffer(frame->current_image_index), view_parameters[view_index].rect };

				for (size_t thread_index = 0; thread_index < view_parameters[view_index].num_threads; ++thread_index) {
					commands.ExecuteCommands(frame->views[view_index].thread_command_buffers);
				}
			}

			//Submit the rendering commands for this frame
			SubmitFrame(*frame);
			return true;
		
		} else {
			return false;
		}
	}
}
