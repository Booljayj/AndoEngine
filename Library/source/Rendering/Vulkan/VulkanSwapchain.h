#pragma once
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanFramework.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"

namespace Rendering {
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

		inline operator bool() const { return !!swapchain; }

		bool Create(CTX_ARG, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical);
		bool Recreate(CTX_ARG, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical);

		void Destroy(VulkanLogicalDevice const& logical);
	};
}
