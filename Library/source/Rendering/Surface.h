#pragma once
#include "Engine/StandardTypes.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	struct RenderingSystem;

	/** A surface tied to a given window, which is used by a RenderSystem for rendering */
	struct Surface {
	public:
		/** The maximum number of consecutive times we can fail to render a frame */
		static constexpr uint8_t maxRetryCount = 5;

		uint32_t id = std::numeric_limits<uint32_t>::max();

		/** The internal surface tied to this surface */
		VkSurfaceKHR surface = nullptr;
		/** The swapchain that is currently being used for images */
		VulkanSwapchain swapchain;
		/** The frame organizer that keeps track of resources used each frame */
		VulkanFrameOrganizer organizer;
		/** The framebuffers for rendering on the swapchain */
		VulkanFramebuffers framebuffers;

		Surface(RenderingSystem const& inOwner, HAL::Window inWindow);

		inline bool operator==(uint32_t otherID) const { return id == otherID; }

		/** Return whether this is a valid surface, which can potentially be used for rendering. It may not be fully set up for rendering yet */
		inline bool IsValidSurface() const { return surface; }
		/** Return whether this is a valid surface that is prepared for rendering */
		inline bool CanRender() const { return surface && swapchain; }
		inline bool IsSwapchainDirty() const { return shouldRecreateSwapchain; }

		/** Get information about how a physical device can be used with this surface */
		VulkanPhysicalDevice GetPhysicalDevice(VkPhysicalDevice device);
		/** Returns whether the physical device can be used to render to this surface */
		bool IsPhysicalDeviceUsable(VulkanPhysicalDevice const& physical) const;
		/** Get the preferred surface format when rendering to this surface with the given physical device */
		VkSurfaceFormatKHR GetPreferredSurfaceFormat(VulkanPhysicalDevice const& physical);

		bool CreateSwapchain(VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes);
		bool RecreateSwapchain(VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes);
		void Destroy(VulkanFramework const& framework, VulkanLogicalDevice const& logical);

		bool Render(VulkanLogicalDevice const& logical, VulkanRenderPasses const& passes, EntityRegistry& registry);

		/** Change the size of this surface */
		void Resize(glm::u32vec2 const& newSize);

		/** Block until pending work is complete */
		void WaitForCompletion(VulkanLogicalDevice const& logical);

	private:
		HAL::Window window;
		RenderingSystem const& owner;
		glm::u32vec2 imageSize;

		uint8_t retryCount : 1;
		uint8_t shouldRecreateSwapchain : 1;
	};
}
