#pragma once
#include "Engine/StandardTypes.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct Surface;
	struct VulkanLogicalDevice;
	struct VulkanPhysicalDevice;

	/**
	 * Contains the components of a Vulkan swapchain, which are used to communicate with the graphics device.
	 * These components are recreated when devices or rendering parameters change (such as screen size).
	 */
	struct VulkanSwapchain {
		VkSwapchainKHR swapchain = nullptr;

		/** The images in the swapchain */
		std::vector<VkImage> images;

		VkSurfaceFormatKHR surfaceFormat = {};
		VkPresentModeKHR presentMode = {};
		glm::u32vec2 extent = { 1, 1 };
		VkSurfaceTransformFlagBitsKHR preTransform = {};

		VulkanSwapchain(VkDevice inDevice, VulkanSwapchain* previous, VulkanPhysicalDevice const& physical, Surface const& surface);
		VulkanSwapchain(VulkanSwapchain const&) = delete;
		VulkanSwapchain(VulkanSwapchain&&) noexcept;
		~VulkanSwapchain();

		inline operator bool() const { return swapchain && images.size() > 0 && std::find(images.begin(), images.end(), VkImage{nullptr}) == images.end(); }
		inline operator VkSwapchainKHR() const { return swapchain; }

		friend void swap(Rendering::VulkanSwapchain& lhs, Rendering::VulkanSwapchain& rhs) {
			std::swap(lhs.swapchain, rhs.swapchain);
			std::swap(lhs.images, rhs.images);
			std::swap(lhs.surfaceFormat, rhs.surfaceFormat);
			std::swap(lhs.presentMode, rhs.presentMode);
			std::swap(lhs.extent, rhs.extent);
			std::swap(lhs.preTransform, rhs.preTransform);
		}

	private:
		/** The logical device that created this swapchain */
		VkDevice device = nullptr;
	};
}
