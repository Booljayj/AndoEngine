#pragma once
#include "Engine/ArrayView.h"
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"

namespace Rendering {
	/** A specific swapchain image and the fence for using it */
	struct VulkanSwapchainImageInfo {
		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkFence fence = nullptr;
	};

	/**
	 * Contains the components of a Vulkan swapchain, which are used to communicate with the graphics device.
	 * These components are recreated when devices or rendering parameters change (such as screen size).
	 */
	struct VulkanSwapchain {
		VkSwapchainKHR swapchain = nullptr;

		VkSurfaceFormatKHR surfaceFormat;
		VkPresentModeKHR presentMode;
		VkExtent2D extent;
		VkSurfaceTransformFlagBitsKHR preTransform;

		std::vector<VulkanSwapchainImageInfo> imageInfos;
		VkRenderPass renderPass;

		std::vector<VkFramebuffer> framebuffers;

		inline operator bool() const { return !!swapchain; }

		static VulkanSwapchain Create(CTX_ARG, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical);
		static VulkanSwapchain Recreate(CTX_ARG, VulkanSwapchain& previous, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical);

		void Destroy(VulkanLogicalDevice const& logical);
	};
}
