#include "Rendering/MaterialComponent.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/Surface.h"
#include "Rendering/Uniforms.h"

namespace Rendering {
	Surface::Surface(CTX_ARG, RenderingSystem const& inOwner, HAL::Window inWindow)
	: id(inWindow.id)
	, window(inWindow)
	, owner(inOwner)
	, retryCount(0)
	, shouldRecreateSwapchain(false)
	{
		// Vulkan surface (via SDL)
		assert(!surface);
#if SDL_ENABLED
		if (SDL_Vulkan_CreateSurface(window.handle, owner.framework.instance, &surface) != SDL_TRUE) {
			LOG(Vulkan, Error, "Failed to create Vulkan window surface");
		}

		int32_t width = 1;
		int32_t height = 1;
		SDL_Vulkan_GetDrawableSize(window.handle, &width, &height);
		imageSize = glm::u32vec2{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
#endif
	}

	VulkanPhysicalDevice Surface::GetPhysicalDevice(CTX_ARG, VkPhysicalDevice device) {
		return VulkanPhysicalDevice::Get(CTX, device, surface);
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

	bool Surface::CreateSwapchain(CTX_ARG, VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes) {
		imageSize = physical.GetSwapExtent(surface, imageSize);

		if (!swapchain.Create(CTX, imageSize, surface, *owner.selectedPhysical, owner.logical)) return false;
		if (!organizer.Create(CTX, *owner.selectedPhysical, owner.logical, owner.uniformLayouts, EBuffering::Double, swapchain.views.size(), 1)) return false;

		SurfaceRenderPass::AttachmentImageViews sharedImageViews;
		if (!passes.surface.CreateFramebuffers(CTX, owner.logical, sharedImageViews, swapchain.views, imageSize, framebuffers.surface)) return false;

		return true;
	}

	bool Surface::RecreateSwapchain(CTX_ARG, VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes) {
		passes.surface.DestroyFramebuffers(owner.logical, framebuffers.surface);

		imageSize = physical.GetSwapExtent(surface, imageSize);

		if (!swapchain.Recreate(CTX, imageSize, surface, *owner.selectedPhysical, owner.logical)) return false;
		if (!organizer.Create(CTX, *owner.selectedPhysical, owner.logical, owner.uniformLayouts, EBuffering::Double, swapchain.views.size(), 1)) return false;

		SurfaceRenderPass::AttachmentImageViews sharedImageViews;
		if (!passes.surface.CreateFramebuffers(CTX, owner.logical, sharedImageViews, swapchain.views, imageSize, framebuffers.surface)) return false;

		return true;
	}

	void Surface::Destroy(VulkanFramework const& framework, VulkanLogicalDevice const& logical) {
		SurfaceRenderPass::DestroyFramebuffers(owner.logical, framebuffers.surface);

		if (organizer) organizer.Destroy(logical);
		if (swapchain) swapchain.Destroy(logical);
		if (surface) vkDestroySurfaceKHR(framework.instance, surface, nullptr);
	}

	bool Surface::Render(CTX_ARG, VulkanLogicalDevice const& logical, VulkanRenderPasses const& passes, EntityRegistry& registry) {
		//@todo This would ideally be done with some acceleration structure that contains a mapping between the pipelines and all of
		//      the geometry that should be drawn with that pipeline, to avoid binding the same pipeline more than once and to strip
		//      out culled geometry.
		auto const renderables = registry.GetView<MeshRendererComponent const>();
		auto const materials = registry.GetView<MaterialComponent const>();
		auto const meshes = registry.GetView<MeshComponent const>();

		//Prepare the frame for rendering, which may need to wait for resources
		EPreparationResult const result = organizer.Prepare(CTX, logical, swapchain, renderables.size());
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
			const SurfaceRenderPass::ScopedRecord scope{ frame.commands, passes.surface, framebuffers.surface[index], Geometry::ScreenRect{ glm::vec2{0.0f}, swapchain.extent } };

			//Set the viewport and scissor that we are currently rendering for.
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapchain.extent.x;
			viewport.height = (float)swapchain.extent.y;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = VkExtent2D{ swapchain.extent.x, swapchain.extent.y };

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
				auto& renderer = renderables.Get<MeshRendererComponent const>(id);

				auto const* material = materials.Find<MaterialComponent const>(renderer.material);
				auto const* mesh = meshes.Find<MeshComponent const>(renderer.mesh);
				if (material && material->resources && mesh && mesh->resources) {
					uint32_t const objectUniformsOffset = sizeof(ObjectUniforms) * objectIndex;

					//Write to the part of the object uniform buffer designated for this object
					ObjectUniforms object;
					object.modelViewProjection = glm::identity<glm::mat4>();
					frame.uniforms.object.ubo.WriteValue(object, objectUniformsOffset);

					//Bind the pipeline that will be used for rendering
					vkCmdBindPipeline(frame.commands, VK_PIPELINE_BIND_POINT_GRAPHICS, material->resources.pipeline);

					//Bind the descriptor sets to use for this draw command
					VkDescriptorSet sets[] = {
						frame.uniforms.global.set,
						//materialInstance->resources.set,
						frame.uniforms.object.set,
					};
					vkCmdBindDescriptorSets(frame.commands, VK_PIPELINE_BIND_POINT_GRAPHICS, material->resources.pipelineLayout, 0, std::size(sets), sets, 1, &objectUniformsOffset);

					//Bind the vetex and index buffers for the mesh
					VkBuffer const vertexBuffers[] = { mesh->resources.buffer };
					VkDeviceSize const vertexOffsets[] = { mesh->resources.offset.vertex };
					vkCmdBindVertexBuffers(frame.commands, 0, 1, vertexBuffers, vertexOffsets);
					vkCmdBindIndexBuffer(frame.commands, mesh->resources.buffer, mesh->resources.offset.index, VK_INDEX_TYPE_UINT32);

					//Submit the command to draw using the vertex and index buffers
					vkCmdDrawIndexed(frame.commands, mesh->resources.size.indices, 1, 0, 0, 0);

					++objectIndex;
				}
			}
		};

		//Record rendering commands for this frame
		if (!organizer.Record(CTX, recorder)) return false;

		//Submit the rendering commands for this frame
		if (!organizer.Submit(CTX, logical, swapchain)) return false;

		retryCount = 0;
		return true;
	}

	void Surface::WaitForCompletion(CTX_ARG, VulkanLogicalDevice const& logical) {
		organizer.WaitForCompletion(CTX, logical);
	}
}