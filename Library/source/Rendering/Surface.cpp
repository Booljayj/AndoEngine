#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Surface.h"
#include "Rendering/UniformTypes.h"

namespace Rendering {
	Surface::Surface(VkInstance instance, HAL::Window& inWindow)
	: instance(instance), window(inWindow), retryCount(0), shouldRecreateSwapchain(false)
	{
		if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE || !surface) {
			throw std::runtime_error{ "Failed to create Vulkan window surface" };
		}

		int32_t width = 1, height = 1;
		SDL_Vulkan_GetDrawableSize(window, &width, &height);
		windowSize = glm::u32vec2{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}

	Surface::~Surface() {
		organizer.reset();
		framebuffers.reset();
		swapchain.reset();
		
		vkDestroySurfaceKHR(instance, surface, nullptr);

		window.destroyed.Remove(windowDestroyedHandle);
	}

	void Surface::InitializeRendering(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& layouts) {
		if (queues || swapchain) throw std::runtime_error{ "Initializing rendering on a surface which is already initialized" };

		t_vector<QueueFamilyDescription> const families = physical.GetSurfaceFamilies(*this);
		
		auto const references = SurfaceQueues::References::Find(families);
		if (!references) {
			LOGF(Vulkan, Warning, "Physical device does not support required queues for surface %i. Cannot initialize rendering.", GetID());
			return;
		}

		queues = references->ResolveFrom(device.queues);
		if (!queues) {
			LOGF(Vulkan, Warning, "Device does not contain required queues for surface %i. Cannot initialize rendering.", GetID());
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
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				swapchain->GetExtent(viewport.width, viewport.height);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				swapchain->GetExtent(scissor.extent.width, scissor.extent.height);
				
				vkCmdSetViewport(commands, 0, 1, &viewport);
				vkCmdSetScissor(commands, 0, 1, &scissor);

				//Write global uniform values that can be accessed by shaders
				GlobalUniforms global;
				global.viewProjection = glm::identity<glm::mat4>();
				global.viewProjectionInverse = glm::inverse(global.viewProjection);
				global.time = 0;
				uniforms.global.Write(global, 0);

				uint32_t objectIndex = 0;
				for (const auto id : renderables) {
					auto const& renderer = renderables.get<MeshRenderer const>(id);

					if (renderer.material && renderer.material->resources && renderer.mesh && renderer.mesh->gpuResources) {
						uint32_t const objectUniformsOffset = static_cast<uint32_t>(sizeof(ObjectUniforms) * objectIndex);

						//Write to the part of the object uniform buffer designated for this object
						ObjectUniforms object;
						object.modelViewProjection = glm::identity<glm::mat4>();
						uniforms.object.Write(object, objectIndex);

						//Bind the pipeline that will be used for rendering
						vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.material->resources.pipeline);

						//Bind the descriptor sets to use for this draw command
						VkDescriptorSet sets[] = {
							uniforms.global,
							uniforms.object,
							//materialInstance->resources.set,
						};
						vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.material->resources.pipelineLayout, 0, static_cast<uint32_t>(std::size(sets)), sets, 1, &objectUniformsOffset);

						//Bind the vetex and index buffers for the mesh
						MeshResources const& meshRes = *renderer.mesh->gpuResources;
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
			organizer->Submit(TArrayView<VkCommandBuffer const>{ context->primaryCommandBuffer });

			retryCount = 0;
			return true;
		}
		else
		{
			if (++retryCount >= maxRetryCount) {
				LOGF(Rendering, Error, "Too many subsequent frames (%i) have failed to render.", maxRetryCount);
				return false;
			}
			return true;
		}
	}
}
