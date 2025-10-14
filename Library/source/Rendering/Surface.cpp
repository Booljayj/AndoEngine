#include "Rendering/Surface.h"
#include "Engine/EnumArray.h"
#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/UniformTypes.h"
#include "Rendering/Vulkan/GraphicsCommands.h"
#include "Rendering/Vulkan/QueueSelection.h"

namespace Rendering {
	Surface::Surface(VkInstance instance, HAL::Window& inWindow)
	: instance(instance), window(inWindow), retryCount(0), shouldRecreateSwapchain(false)
	{
		if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE || !surface) {
			throw std::runtime_error{ "Failed to create Vulkan window surface" };
		}

		glm::i32vec2 drawableSize = { 1, 1 };
		SDL_Vulkan_GetDrawableSize(window, &drawableSize.x, &drawableSize.y);
		windowSize = glm::u32vec2{ static_cast<uint32_t>(drawableSize.x), static_cast<uint32_t>(drawableSize.y) };
	}

	Surface::~Surface() {
		organizer.reset();
		framebuffers.reset();
		swapchain.reset();
		
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	void Surface::InitializeRendering(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& layouts) {
		if (queues || swapchain) throw std::runtime_error{ "Initializing rendering on a surface which is already initialized" };

		QueueFamilySelectors selectors{ physical.GetSurfaceFamilies(*this) };
		
		auto const references = selectors.SelectSurfaceQueues();
		if (!references) {
			LOG(Vulkan, Warning, "Physical device does not support required queues for surface {}. Cannot initialize rendering.", GetID());
			return;
		}

		queues = device.queues.Resolve(*references);
		if (!queues) {
			LOG(Vulkan, Warning, "Device does not contain required queues for surface {}. Cannot initialize rendering.", GetID());
			return;
		}

		PhysicalDevicePresentation const presentation = PhysicalDevicePresentation::GetPresentation(physical, *this).value();
		PhysicalDeviceCapabilities const capabilities{ physical, *this };

		swapchain.emplace(device, nullptr, presentation, capabilities, *this);
		framebuffers.emplace(device, *swapchain, passes);
		organizer.emplace(device, device, *queues, *swapchain, layouts, EBuffering::Double);
	}

	void Surface::DeinitializeRendering() {
		organizer.reset();
		framebuffers.reset();
		swapchain.reset();
		queues.reset();
	}

	bool Surface::RecreateSwapchain(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& layouts) {
		organizer.reset();
		framebuffers.reset();

		PhysicalDevicePresentation const presentation = PhysicalDevicePresentation::GetPresentation(physical, *this).value();
		PhysicalDeviceCapabilities const capabilities{ physical, *this };

		if (swapchain.has_value()) {
			//The previous swapchain needs to exist long enough to create the new one, so we create a temporary new value before assigning it.
			auto recreated = std::make_optional<Swapchain>(device, &swapchain.value(), presentation, capabilities, *this);
			swapchain.swap(recreated);
		} else {
			swapchain.emplace(device, nullptr, presentation, capabilities, *this);
		}

		framebuffers.emplace(device, *swapchain, passes);
		organizer.emplace(device, device, *queues, *swapchain, layouts, EBuffering::Double);

		return true;
	}

	bool Surface::Render(RenderPasses const& passes, entt::registry& registry, ResourcesCollection& previous_resources) {
		//@todo This would ideally be done with some acceleration structure that contains a mapping between the pipelines and all of
		//      the geometry that should be drawn with that pipeline, to avoid binding the same pipeline more than once and to strip
		//      out culled geometry.
		
		auto const renderables = registry.view<MeshRenderer const>();

		constexpr size_t numThreads = 1;
		auto const context = organizer->CreateRecordingContext(renderables.size(), numThreads, previous_resources);
		if (context) {
			{
				FrameUniforms& uniforms = context->uniforms;

				glm::u32vec2 const extent = swapchain->GetExtent();
				VkViewport const viewport{
					.x = 0.0f,
					.y = 0.0f,
					.width = static_cast<float>(extent.x),
					.height = static_cast<float>(extent.y),
					.minDepth = 0.0f,
					.maxDepth = 1.0f,
				};
				VkRect2D const scissor{
					.offset = { 0, 0 },
					.extent = { extent.x, extent.y },
				};

				//The inheritance info that is shared by all secondary command buffers
				CommandInheritance const inheritance{
					.renderPass = passes.surface,
					.framebuffer = framebuffers->surface[context->imageIndex],
				};
				
				const auto draw_partial = [&passes, renderables, &context, &uniforms, &viewport, &scissor, &inheritance](size_t thread_index) {
					ThreadBuffer buffer{ 20'000 };

					size_t const objects_per_thread = renderables.size() / numThreads;
					size_t const extra_objects = renderables.size() % numThreads;

					size_t const start = thread_index * objects_per_thread;
					size_t const end = start + objects_per_thread + (thread_index < extra_objects ? 1 : 0);

					GraphicsCommandWriter commands{ context->secondaryCommandBuffers[thread_index], inheritance };

					commands.SetViewports(0, MakeSpan(viewport));
					commands.SetScissors(0, MakeSpan(scissor));

					for (size_t index = start; index < end; ++index) {
						entt::entity const entity = renderables.handle()->operator[](index);
						auto const& renderer = renderables.get<MeshRenderer const>(entity);

						Material const* material = renderer.material.get();
						StaticMesh const* mesh = renderer.mesh.get();

						if (material && material->objects && mesh && mesh->objects) {
							context->threadResources[thread_index] += material->objects;
							context->threadResources[thread_index] += mesh->objects;

							enum class EDynamicOffsets : uint8_t {
								Object,
								MAX
							};
							EnumArray<uint32_t, EDynamicOffsets> offsets;

							//Write to the part of the object uniform buffer designated for this object
							offsets[EDynamicOffsets::Object] = uniforms.object.Write(
								ObjectUniforms{
									.modelViewProjection = glm::identity<glm::mat4>(),
								},
								index
							);

							//Bind the pipeline that will be used for rendering
							commands.BindGraphicsPipeline(material->objects->pipeline);
							
							//Bind the descriptor sets to use for this draw command
							EnumArray<VkDescriptorSet, EGraphicsLayouts> sets;
							sets[EGraphicsLayouts::Global] = uniforms.global;
							sets[EGraphicsLayouts::Object] = uniforms.object;
							//sets[EGraphicsLayouts::Material] = material->gpuResources->set;

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

				//Start the worker threads that will begin adding commands to the secondary command buffers. We want this to happen in parallel with as much of the main thread work as possible.
				t_vector<std::jthread> workers;
				workers.reserve(numThreads);

				for (size_t thread_index = 0; thread_index < numThreads; ++thread_index) {
					workers.emplace_back(draw_partial, thread_index);
				}

				//Write global uniform values that can be accessed by shaders
				uniforms.global.Write(
					GlobalUniforms{
						.viewProjection = glm::identity<glm::mat4>(),
						.viewProjectionInverse = glm::inverse(glm::identity<glm::mat4>()),
						.time = 0,
					}
				);

				GraphicsCommandWriter commands{ context->primaryCommandBuffer };

				//Create a scope for all commands that belong to the surface render pass
				Geometry::ScreenRect const rect = { glm::i32vec2{ 0, 0 }, extent };
				SurfaceRenderPass::ScopedRecord const scopedRecord{ commands, passes.surface, framebuffers->surface[context->imageIndex], rect };

				//Wait for the workers to finish, and then add their commands to the primary command buffer. We do this by clearing to make sure workers are not used beyond this point.
				workers.clear();
				commands.ExecuteCommands(context->secondaryCommandBuffers);
			}

			//Submit the rendering commands for this frame
			organizer->Submit(*context);

			retryCount = 0;
			return true;

		} else {
			if (++retryCount >= maxRetryCount) {
				LOG(Rendering, Error, "Too many subsequent frames ({}) have failed to render.", maxRetryCount);
				return false;
			}
			return true;
		}
	}
}
