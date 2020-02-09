#pragma once
#include <vulkan/vulkan.h>
#include "Engine/ArrayView.h"
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"

namespace Rendering {
	/** A specific swapchain image and the fence for using it */
	struct VulkanSwapchainImage {
		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkFence fence = nullptr;
	};

	struct VulkanSwapchain {
		VkSwapchainKHR swapchain = nullptr;

		VkSurfaceFormatKHR surfaceFormat;
		VkPresentModeKHR presentMode;
		VkExtent2D extent;
		VkSurfaceTransformFlagBitsKHR preTransform;

		std::vector<VulkanSwapchainImage> images;

		bool Create(CTX_ARG, VkExtent2D const& desiredExtent, VkSurfaceKHR const& surface, Rendering::VulkanPhysicalDevice const& physicalDevice, VkDevice const& logicalDevice);
		void Destroy(VkDevice const& logicalDevice);

	private:
		static uint32_t GetImageCountMinimum(VkSurfaceCapabilitiesKHR const& capabilities);
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableSurfaceFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes);

		static void DestroyResources(VkDevice const& device, VkSwapchainKHR const& swapchain, TArrayView<VulkanSwapchainImage const> const& images);
	};
}
