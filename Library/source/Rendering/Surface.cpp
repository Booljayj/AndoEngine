#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Surface.h"
#include "Rendering/Uniforms.h"

namespace Rendering {
	Surface::Surface(RenderingSystem& inOwner, HAL::Window& inWindow)
	: window(inWindow)
	, owner(inOwner)
	, retryCount(0)
	, shouldRecreateSwapchain(false)
	{
		// Vulkan surface (via SDL)
		if (SDL_Vulkan_CreateSurface(window, owner.framework.instance, &surface) != SDL_TRUE) {
			LOG(Vulkan, Error, "Failed to create Vulkan window surface");
		}

		int32_t width = 1, height = 1;
		SDL_Vulkan_GetDrawableSize(window, &width, &height);
		windowSize = glm::u32vec2{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		imageSize = windowSize;
	}

	Surface::~Surface() {
		window.destroyed.Remove(windowDestroyedHandle);
		windowDestroyedHandle.reset();
	}

	VulkanPhysicalDevice Surface::GetPhysicalDevice(VkPhysicalDevice device) {
		return VulkanPhysicalDevice::Get(device, surface);
	}

	bool Surface::IsPhysicalDeviceUsable(VulkanPhysicalDevice const& physical) const {
		return physical.HasRequiredQueues() && physical.HasSwapchainSupport();
	}

	VkSurfaceFormatKHR Surface::GetPreferredSurfaceFormat(VulkanPhysicalDevice const& physical) {
		for (const auto& availableSurfaceFormat : physical.presentation.surfaceFormats) {
			if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableSurfaceFormat;
			}
		}
		return physical.presentation.surfaceFormats[0];
	}

	bool Surface::CreateSwapchain(VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes) {
		imageSize = physical.GetSwapExtent(surface, windowSize);

		swapchain.emplace(owner.logical.device, nullptr, *owner.selectedPhysical, *this);
		framebuffers.emplace(owner.logical, *swapchain, passes);

		if (!organizer.Create(*owner.selectedPhysical, owner.logical, owner.uniformLayouts, EBuffering::Double, swapchain->images.size(), 1)) return false;

		return true;
	}

	bool Surface::RecreateSwapchain(VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes) {
		imageSize = physical.GetSwapExtent(surface, windowSize);

		framebuffers.reset();

		if (swapchain.has_value()) {
			//The previous swapchain needs to exist long enough to create the new one, so we create a temporary new value before assigning it.
			auto recreated = std::make_optional<VulkanSwapchain>(owner.logical.device, &swapchain.value(), *owner.selectedPhysical, *this);
			swapchain.swap(recreated);
		} else {
			swapchain.emplace(owner.logical.device, nullptr, *owner.selectedPhysical, *this);
		}

		framebuffers.emplace(owner.logical, *swapchain, passes);

		if (!organizer.Create(*owner.selectedPhysical, owner.logical, owner.uniformLayouts, EBuffering::Double, swapchain->images.size(), 1)) return false;

		return true;
	}

	void Surface::Destroy(VulkanFramework const& framework, VulkanLogicalDevice const& logical) {
		if (organizer) organizer.Destroy(logical);
		framebuffers.reset();
		swapchain.reset();
		if (surface) vkDestroySurfaceKHR(framework.instance, surface, nullptr);
	}

	bool Surface::Render(VulkanLogicalDevice const& logical, VulkanRenderPasses const& passes, entt::registry& registry) {
		//@todo This would ideally be done with some acceleration structure that contains a mapping between the pipelines and all of
		//      the geometry that should be drawn with that pipeline, to avoid binding the same pipeline more than once and to strip
		//      out culled geometry.
		auto const renderables = registry.view<MeshRenderer const>();

		//Prepare the frame for rendering, which may need to wait for resources
		EPreparationResult const result = organizer.Prepare(logical, *swapchain, renderables.size());
		if (result == EPreparationResult::Retry) {
			if (++retryCount >= maxRetryCount) {
				LOGF(Rendering, Error, "Too many subsequent frames (%i) have failed to render.", maxRetryCount);
				return false;
			}
			return true;
		}
		if (result == EPreparationResult::Error) return false;

		//Lambda to record rendering commands
		auto const recorder = [&](const FrameResources& frame, size_t index) {
			//Create a scope for all commands that belong to the surface render pass
			const SurfaceRenderPass::ScopedRecord scope{ frame.commands, passes.surface, framebuffers->surface[index], Geometry::ScreenRect{ glm::vec2{0.0f}, swapchain->extent } };

			//Set the viewport and scissor that we are currently rendering for.
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapchain->extent.x;
			viewport.height = (float)swapchain->extent.y;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = VkExtent2D{ swapchain->extent.x, swapchain->extent.y };

			vkCmdSetViewport(frame.commands, 0, 1, &viewport);
			vkCmdSetScissor(frame.commands, 0, 1, &scissor);

			//Write global uniform values that can be accessed by shaders
			GlobalUniforms global;
			global.viewProjection = glm::identity<glm::mat4>();
			global.viewProjectionInverse = glm::inverse(global.viewProjection);
			global.time = 0;
			frame.uniforms.global.ubo.WriteValue(global, 0);

			uint32_t objectIndex = 0;
			for (const auto id : renderables) {
				auto const& renderer = renderables.get<MeshRenderer const>(id);

				if (renderer.material && renderer.material->resources && renderer.mesh && renderer.mesh->gpuResources) {
					uint32_t const objectUniformsOffset = static_cast<uint32_t>(sizeof(ObjectUniforms) * objectIndex);

					//Write to the part of the object uniform buffer designated for this object
					ObjectUniforms object;
					object.modelViewProjection = glm::identity<glm::mat4>();
					frame.uniforms.object.ubo.WriteValue(object, objectUniformsOffset);

					//Bind the pipeline that will be used for rendering
					vkCmdBindPipeline(frame.commands, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.material->resources.pipeline);

					//Bind the descriptor sets to use for this draw command
					VkDescriptorSet sets[] = {
						frame.uniforms.global.set,
						//materialInstance->resources.set,
						frame.uniforms.object.set,
					};
					vkCmdBindDescriptorSets(frame.commands, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.material->resources.pipelineLayout, 0, static_cast<uint32_t>(std::size(sets)), sets, 1, &objectUniformsOffset);

					//Bind the vetex and index buffers for the mesh
					VulkanMeshResources const& meshRes = renderer.mesh->gpuResources;
					VkBuffer const vertexBuffers[] = { meshRes.buffer };
					VkDeviceSize const vertexOffsets[] = { meshRes.offset.vertex };
					vkCmdBindVertexBuffers(frame.commands, 0, 1, vertexBuffers, vertexOffsets);
					vkCmdBindIndexBuffer(frame.commands, meshRes.buffer, meshRes.offset.index, meshRes.indexType);

					//Submit the command to draw using the vertex and index buffers
					vkCmdDrawIndexed(frame.commands, meshRes.size.indices, 1, 0, 0, 0);

					++objectIndex;
				}
			}
		};

		//Record rendering commands for this frame
		if (!organizer.Record(recorder)) return false;

		//Submit the rendering commands for this frame
		if (!organizer.Submit(logical, *swapchain)) return false;

		retryCount = 0;
		return true;
	}

	void Surface::WaitForCompletion(VulkanLogicalDevice const& logical) {
		organizer.WaitForCompletion(logical);
	}
}
