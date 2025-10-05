#include "Rendering/Surface.h"
#include "Engine/EnumArray.h"
#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/UniformTypes.h"
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

	bool Surface::Render(RenderPasses const& passes, entt::registry& registry) {
		//@todo This would ideally be done with some acceleration structure that contains a mapping between the pipelines and all of
		//      the geometry that should be drawn with that pipeline, to avoid binding the same pipeline more than once and to strip
		//      out culled geometry.
		
		auto const renderables = registry.view<MeshRenderer const>();

		constexpr size_t numThreads = 1;
		auto const context = organizer->CreateRecordingContext(renderables.size(), numThreads);
		if (context) {
			FrameUniforms& uniforms = context->uniforms;
			VkCommandBuffer const commands = context->primaryCommandBuffer;

			Framebuffer const& surfaceFramebuffer = framebuffers->surface[context->imageIndex];
			Geometry::ScreenRect const rect = { glm::vec2{ 0.0f, 0.0f }, swapchain->GetExtent() };

			{
				//Scope within which we will write commands to the buffer
				ScopedCommands const scopedCommands{ commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
				//Create a scope for all commands that belong to the surface render pass
				SurfaceRenderPass::ScopedRecord const scopedRecord{ commands, passes.surface, surfaceFramebuffer, rect };

				//Set the viewport and scissor that we are currently rendering for.
				{
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
					vkCmdSetViewport(commands, 0, 1, &viewport);
					vkCmdSetScissor(commands, 0, 1, &scissor);
				}

				//Write global uniform values that can be accessed by shaders
				uniforms.global.Write(
					GlobalUniforms{
						.viewProjection = glm::identity<glm::mat4>(),
						.viewProjectionInverse = glm::inverse(glm::identity<glm::mat4>()),
						.time = 0,
					}
				);

				uint32_t objectIndex = 0;
				for (const auto id : renderables) {
					auto const& renderer = renderables.get<MeshRenderer const>(id);

					Material const* material = renderer.material.get();
					StaticMesh const* mesh = renderer.mesh.get();

					if (material && material->objects && mesh && mesh->objects) {
						context->objects.push_back(material->objects);
						context->objects.push_back(mesh->objects);

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
							objectIndex
						);

						//Bind the pipeline that will be used for rendering
						vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, material->objects->pipeline);

						//Bind the descriptor sets to use for this draw command
						EnumArray<VkDescriptorSet, EGraphicsLayouts> sets;
						sets[EGraphicsLayouts::Global] = uniforms.global;
						sets[EGraphicsLayouts::Object] = uniforms.object;
						//sets[EGraphicsLayouts::Material] = material->gpuResources->set;

						vkCmdBindDescriptorSets(
							commands, VK_PIPELINE_BIND_POINT_GRAPHICS, material->objects->layouts.pipeline,
							0, sets.size(), sets.data(), offsets.size(), offsets.data()
						);

						//Bind the vetex and index buffers for the mesh
						MeshResources const& meshRes = *mesh->objects;
						VkBuffer const vertexBuffers[] = { meshRes.buffer };
						VkDeviceSize const vertexOffsets[] = { meshRes.offset.vertex };
						vkCmdBindVertexBuffers(commands, 0, 1, vertexBuffers, vertexOffsets);
						vkCmdBindIndexBuffer(commands, meshRes.buffer, meshRes.offset.index, meshRes.indexType);

						//Submit the command to draw using the vertex and index buffers
						vkCmdDrawIndexed(commands, meshRes.size.indices, 1, 0, 0, 0);

						++objectIndex;
					}
				}
			}

			//Submit the rendering commands for this frame
			organizer->Submit(std::span<VkCommandBuffer const>{ &context->primaryCommandBuffer, 1 });

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

	RenderObjectsHandleCollection& operator<<(RenderObjectsHandleCollection& collection, Surface& surface) {
		if (surface.organizer) collection << *surface.organizer;
		return collection;
	}
}
