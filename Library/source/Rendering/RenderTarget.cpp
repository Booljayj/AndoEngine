#include "Rendering/RenderTarget.h"
#include "Engine/GLM.h"
#include "Engine/Threads.h"
#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/RenderTargetContexts.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Views/View.h"
#include "Rendering/Vulkan/GraphicsQueue.h"
#include "Rendering/Vulkan/PushConstants.h"
#include "Rendering/Vulkan/RenderPasses.h"
#include "Rendering/Vulkan/Resources.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "ThirdParty/EnTT.h"

namespace Rendering {
	DEFINE_LOG_CATEGORY_MEMBER(RenderTarget, RenderTarget, Debug);

	using CullingThreadResults = ThreadSafe<std::vector<StaticMeshParameters const*>>;
	using RecordingThreadResults = ThreadSafe<std::vector<VkCommandBuffer>>;

	void PerformThreadCulling(ViewParameters const& view_parameters, ViewContext& view, size_t thread_index, CullingThreadResults& ts_results) {
		ThreadBuffer buffer{ 20'000 };

		auto const renderables = view_parameters.registry->view<MeshRenderer const>();

		size_t const objects_per_thread = renderables.size() / view_parameters.num_culling_threads;
		size_t const extra_objects = renderables.size() % view_parameters.num_culling_threads;

		size_t const start = thread_index * objects_per_thread + (thread_index < extra_objects ? thread_index : extra_objects);
		size_t const end = start + objects_per_thread + static_cast<size_t>(thread_index < extra_objects);

		ThreadMeshCollection& mesh_collection = view.culling.thread_mesh_collections[thread_index];

		const auto* entities = renderables.handle();
		for (size_t index = start; index < end; ++index) {
			entt::entity const entity = entities->operator[](index);

			MeshRenderer const& renderer = renderables[entity];
			Material const* material = renderer.material.get();
			StaticMesh const* mesh = renderer.mesh.get();

			if (material && material->objects && mesh && mesh->objects) {
				mesh_collection.static_meshes.emplace_back(material->objects, mesh->objects);
			}
		}

		//This thread is finshed culling, so lock the results and dump what was collected. Each thread dumps its own results as they finish.
		auto results = ts_results.LockExclusive();
		results->append_range(ranges::views::transform(mesh_collection.static_meshes, [](StaticMeshParameters const& element) { return &element; }));
	}

	void PerformThreadRecording(ViewParameters const& view_parameters, ViewContext& view, std::span<StaticMeshParameters const* const> static_meshes, CommandInheritance const& inheritance, size_t thread_index, RecordingThreadResults& ts_results) {
		ThreadBuffer buffer{ 20'000 };

		size_t const objects_per_thread = static_meshes.size() / view_parameters.num_recording_threads;
		size_t const extra_objects = static_meshes.size() % view_parameters.num_recording_threads;

		size_t const start = thread_index * objects_per_thread + (thread_index < extra_objects ? thread_index : extra_objects);
		size_t const end = start + objects_per_thread + static_cast<size_t>(thread_index < extra_objects);

		GraphicsCommandWriter const commands{ view.recording.thread_command_buffers[thread_index], inheritance };

		VkViewport const viewport{
			.x = static_cast<float>(view_parameters.rect.offset.x),
			.y = static_cast<float>(view_parameters.rect.offset.y),
			.width = static_cast<float>(view_parameters.rect.extent.x),
			.height = static_cast<float>(view_parameters.rect.extent.y),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		VkRect2D const scissor{
			.offset = { view_parameters.rect.offset.x, view_parameters.rect.offset.y },
			.extent = { view_parameters.rect.extent.x, view_parameters.rect.extent.y },
		};

		commands.SetViewports(0, MakeSpan(viewport));
		commands.SetScissors(0, MakeSpan(scissor));

		for (size_t index = start; index < end; ++index) {
			StaticMeshParameters const* static_mesh = static_meshes[index];

			//Write this object's data into the object buffer, and keep track of the index for it
			uint32_t const object_buffer_index = view.recording.object_uniforms.Write(
				ObjectUniforms{
					.modelViewProjection = glm::identity<glm::mat4>(),
				},
				index
			);

			//Bind the pipeline that will be used for rendering
			commands.BindGraphicsPipeline(static_mesh->pipeline_resources->pipeline);

			//Bind the descriptor sets to use for this draw command
			EnumArray<VkDescriptorSet, EGraphicsLayouts> sets;
			sets[EGraphicsLayouts::Global] = view.recording.global_uniforms;
			sets[EGraphicsLayouts::Object] = view.recording.object_uniforms;
			//sets[EGraphicsLayouts::Material] = material->gpuResources->set;

			enum class EDynamicOffsets {
				Object,
				MAX,
			};
			EnumArray<uint32_t, EDynamicOffsets> offsets;
			offsets[EDynamicOffsets::Object] = object_buffer_index;
			//offsets[EDynamicOffsets::Material] = ???

			commands.BindGraphicsDescriptorSets(static_mesh->pipeline_resources->layouts.pipeline, 0, MakeSpan(sets), MakeSpan(offsets));

			//Bind the vertex stage push constants for this object, including the vertex buffer (for Vertex Pulling)
			MeshPushConstants const push_constants{
				.mvp_matrix = glm::identity<glm::mat4>(),
				.buffer_address = static_mesh->mesh_resources->address + static_mesh->mesh_resources->offset.vertex,
			};
			commands.PushConstants(static_mesh->pipeline_resources->layouts.pipeline, VK_SHADER_STAGE_VERTEX_BIT, 0, push_constants);

			//Bind the index buffer for the mesh
			VkBuffer const mesh_buffer = static_mesh->mesh_resources->buffer;
			commands.BindIndexBuffer(mesh_buffer, static_mesh->mesh_resources->offset.index, static_mesh->mesh_resources->indexType);

			//Submit the command to draw using the vertex and index buffers
			constexpr uint32_t instance_count = 1;
			commands.DrawIndexed(0, static_mesh->mesh_resources->size.indices, 0, instance_count);
		}

		auto results = ts_results.LockExclusive();
		results->emplace_back(view.recording.thread_command_buffers[thread_index]);
	}

	void PerformViewRendering(ViewParameters const& view_parameters, ViewContext& view, SurfaceRenderPass const& render_pass, Framebuffer const& framebuffer, VkCommandBuffer command_buffer) {
		std::vector<std::jthread> thread_workers;

		//Perform culling
		CullingThreadResults ts_static_meshes;
		{
			thread_workers.reserve(view_parameters.num_culling_threads);
		
			for (size_t thread_index = 0; thread_index < view_parameters.num_culling_threads; ++thread_index) {
				thread_workers.emplace_back(
					[&view_parameters, &view, thread_index, &ts_static_meshes]()
					{
						PerformThreadCulling(view_parameters, view, thread_index, ts_static_meshes);
					}
				);
			}

			//Wait for culling to finish before proceeding.
			thread_workers.clear();
		}

		//Perform recording
		RecordingThreadResults ts_prepared_command_buffers;
		{
			//The inheritance info that is shared by all secondary command buffers
			CommandInheritance const inheritance{
				.renderPass = render_pass,
				.framebuffer = framebuffer,
			};

			const auto static_meshes = ts_static_meshes.LockInclusive();

			thread_workers.reserve(view_parameters.num_recording_threads);

			for (size_t thread_index = 0; thread_index < view_parameters.num_recording_threads; ++thread_index) {
				thread_workers.emplace_back(
					[&view_parameters, &view, &static_meshes = *static_meshes, &inheritance, thread_index, &ts_prepared_command_buffers]()
					{
						PerformThreadRecording(view_parameters, view, static_meshes, inheritance, thread_index, ts_prepared_command_buffers);
					}
				);
			}

			glm::mat4 const view_matrix = view_parameters.camera.transform;
			glm::mat4 projection_matrix = glm::perspective(glm::radians(view_parameters.camera.fov), view_parameters.camera.aspect, view_parameters.camera.clip.near, view_parameters.camera.clip.far);
			projection_matrix[1][1] *= -1.0f; //flip this coordinate to account for differences in the Y-axis between OpenGL and Vulkan.
			glm::mat4 const view_projection_matrix = projection_matrix * view_matrix;

			view.recording.global_uniforms.Write(
				GlobalUniforms{
					.viewProjection = view_projection_matrix,
					.viewProjectionInverse = glm::inverse(view_projection_matrix),
					.time = 0,
				}
				);

			//Wait for recording to finish before proceeding
			thread_workers.clear();
		}

		//After the threads are done recording the secondary command buffers, record them to the primary command buffer
		{
			GraphicsCommandWriter const commands{ command_buffer };

			//Create a scope for all commands that belong to the surface render pass
			SurfaceRenderPass::ScopedRecord const scopedRecord{ commands, render_pass, framebuffer, view_parameters.rect };

			auto const prepared_command_buffers = ts_prepared_command_buffers.LockInclusive();
			commands.ExecuteCommands(*prepared_command_buffers);
		}
	}

	bool RenderTarget::Render(RenderPasses const& passes, ResourcesCollection& previous_resources) {
		//Copy initial parameters from the views
		{
			views_parameters.clear();

			auto const views = ts_views.LockInclusive();

			views_parameters.reserve(views->size());
			for (size_t index = 0; index < views->size(); ++index) {
				views_parameters.emplace_back(views[index], extent);
			}
		}

		FrameContext* frame = GetNextFrameContext();
		if (!frame) {
			LOG(RenderTarget, Warning, "Unable to get an available context to use for rendering");
			return false;
		}

		frame->Prepare(views_parameters, previous_resources);

		std::vector<std::jthread> view_workers;
		view_workers.reserve(views_parameters.size());

		auto const& framebuffer = GetFramebuffer(frame->current_image_index);

		for (size_t view_index = 0; view_index < views_parameters.size(); ++view_index) {
			view_workers.emplace_back(
				[&view_parameters = views_parameters[view_index], &view = frame->views[view_index], &render_pass = passes.surface, &framebuffer, command_buffer = frame->view_command_buffers[view_index]]()
				{
					PerformViewRendering(view_parameters, view, render_pass, framebuffer, command_buffer);
				}
			);
		}

		//Wait for all views to finish before proceeding
		view_workers.clear();

		for (size_t view_index = 0; view_index < views_parameters.size(); ++view_index) {
			frame->views[view_index].recording.global_uniforms.Flush();
			frame->views[view_index].recording.object_uniforms.Flush();
		}

		//Submit the frame
		SubmitFrameContext(*frame);
		return true;
	}
}
