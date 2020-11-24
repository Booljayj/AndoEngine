#include "Engine/LinearContainers.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	namespace VulkanSwapchainUtilities {
		uint32_t GetImageCountMinimum(VkSurfaceCapabilitiesKHR const& capabilities) {
			uint32_t const maxImageCountActual = capabilities.maxImageCount > 0 ? capabilities.maxImageCount : std::numeric_limits<uint32_t>::max();
			return std::min<uint32_t>(capabilities.minImageCount + 1, maxImageCountActual);
		}

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats) {
			for (const auto& availableSurfaceFormat : availableSurfaceFormats) {
				if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return availableSurfaceFormat;
				}
			}
			return availableSurfaceFormats[0];
		}

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					return availablePresentMode;
				}
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		}

		void SetupProperties(CTX_ARG, VulkanSwapchain& result, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical) {
			result.surfaceFormat = ChooseSwapSurfaceFormat(physical.presentation.surfaceFormats);
			result.presentMode = ChooseSwapPresentMode(physical.presentation.presentModes);
			result.extent = physical.GetSwapExtent(surface, extent);
			result.preTransform = physical.GetPreTransform(surface);
		}

		bool CreateSwapchain(CTX_ARG, VulkanSwapchain& result, VkSwapchainKHR const& previous, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical) {
			uint32_t const imageCountMinimum = GetImageCountMinimum(physical.presentation.capabilities);

			VkSwapchainCreateInfoKHR swapchainCI = {};
			swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCI.surface = surface;

			//Image settings
			swapchainCI.minImageCount = imageCountMinimum;
			swapchainCI.imageFormat = result.surfaceFormat.format;
			swapchainCI.imageColorSpace = result.surfaceFormat.colorSpace;
			swapchainCI.imageExtent = result.extent;
			swapchainCI.imageArrayLayers = 1;
			swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			//Queue settings
			uint32_t const queueFamilyIndices[2] = {physical.queues.graphics->index, physical.queues.present->index};
			if (queueFamilyIndices[0] == queueFamilyIndices[1]) {
				swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchainCI.queueFamilyIndexCount = 0;
				swapchainCI.pQueueFamilyIndices = nullptr;
			} else {
				swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapchainCI.queueFamilyIndexCount = 2;
				swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
			}

			//Transform settings
			swapchainCI.preTransform = result.preTransform;

			//Compositing settings
			swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

			//Presentation settings
			swapchainCI.presentMode = result.presentMode;
			swapchainCI.clipped = true;

			//The old swapchain that is being replaced, which will be destroyed after the new one is created.
			swapchainCI.oldSwapchain = previous;

			assert(!result.swapchain);
			return vkCreateSwapchainKHR(logical.device, &swapchainCI, nullptr, &result.swapchain) == VK_SUCCESS;
		}

		bool CreateImageViews(CTX_ARG, VulkanSwapchain& result, VulkanLogicalDevice const& logical) {
			TEMP_ALLOCATOR_MARK();

			//Get the raw images from the swapchain
			uint32_t numImages = 0;
			vkGetSwapchainImagesKHR(logical.device, result.swapchain, &numImages, nullptr);
			VkImage* images = CTX.temp.Request<VkImage>(numImages);
			vkGetSwapchainImagesKHR(logical.device, result.swapchain, &numImages, images);

			if (numImages == 0) {
				LOG(Vulkan, Error, "Swapchain has no images to retrieve");
				return false;
			}

			result.views.resize(numImages);
			for (uint32_t index = 0; index < numImages; ++index) {
				//Create image view
				VkImageViewCreateInfo viewCI{};
				viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewCI.image = images[index];
				//Image data settings
				viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewCI.format = result.surfaceFormat.format;
				//Component swizzling settings
				viewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				viewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				//Image usage settings
				viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewCI.subresourceRange.baseMipLevel = 0;
				viewCI.subresourceRange.levelCount = 1;
				viewCI.subresourceRange.baseArrayLayer = 0;
				viewCI.subresourceRange.layerCount = 1;

				assert(!result.views[index]);
				if (vkCreateImageView(logical.device, &viewCI, nullptr, &result.views[index]) != VK_SUCCESS) {
					LOGF(Vulkan, Error, "Failed to create image view %i", index);
					return false;
				}
			}

			return true;
		}
	}

	bool VulkanSwapchain::Create(CTX_ARG, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical) {
		using namespace VulkanSwapchainUtilities;
		TEMP_ALLOCATOR_MARK();

		vkDeviceWaitIdle(logical.device);

		SetupProperties(CTX, *this, extent, surface, physical);

		if (!CreateSwapchain(CTX, *this, nullptr, surface, physical, logical) || !CreateImageViews(CTX, *this, logical)) {
			LOG(Vulkan, Error, "Failed to create swapchain");
			return false;
		}

		return true;
	}

	bool VulkanSwapchain::Recreate(CTX_ARG, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical) {
		using namespace VulkanSwapchainUtilities;
		TEMP_ALLOCATOR_MARK();

		vkDeviceWaitIdle(logical.device);

		SetupProperties(CTX, *this, extent, surface, physical);

		//Store the previous swapchain, and destroy the previous image views
		VkSwapchainKHR previous = swapchain;
		swapchain = nullptr;
		for (VkImageView view : views) {
			if (view) vkDestroyImageView(logical.device, view, nullptr);
		}
		views.clear();

		const bool bSuccess = CreateSwapchain(CTX, *this, previous, surface, physical, logical) && CreateImageViews(CTX, *this, logical);

		//Clean up the previous swapchain regardless of whether the creation of the new one is a success.
		if (previous) vkDestroySwapchainKHR(logical.device, previous, nullptr);

		if (!bSuccess) {
			LOG(Vulkan, Error, "Failed to recreate swapchain");
			return false;
		}

		return true;
	}

	void VulkanSwapchain::Destroy(VulkanLogicalDevice const& logical) {
		for (VkImageView view : views) {
			if (view) vkDestroyImageView(logical.device, view, nullptr);
		}
		if (swapchain) vkDestroySwapchainKHR(logical.device, swapchain, nullptr);
		views.clear();
		swapchain = nullptr;
	}
}
