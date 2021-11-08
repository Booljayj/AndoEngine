#pragma once
#include "Geometry/GLM.h"
#include "Rendering/Vulkan/Vulkan.h"
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

		VkSurfaceFormatKHR surfaceFormat = {};
		VkPresentModeKHR presentMode = {};
		glm::u32vec2 extent;
		VkSurfaceTransformFlagBitsKHR preTransform = {};

		/** The views for images in the swapchain */
		std::vector<VkImageView> views;

		inline operator bool() const { return swapchain && views.size() > 0 && std::find(views.begin(), views.end(), VkImageView{nullptr}) == views.end(); }

		bool Create(glm::u32vec2 const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical);
		bool Recreate(glm::u32vec2 const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical);

		void Destroy(VulkanLogicalDevice const& logical);
	};
}
