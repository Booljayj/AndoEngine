#include "Engine/LinearContainers.h"
#include "Engine/ScopedTempBlock.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	bool VulkanSwapchain::Create(CTX_ARG, VkExtent2D const& desiredExtent, VkSurfaceKHR const& surface, Rendering::VulkanPhysicalDevice const& physicalDevice, VkDevice const& logicalDevice) {
		TEMP_SCOPE;
		bool bSuccess = true;

		surfaceFormat = ChooseSwapSurfaceFormat(physicalDevice.presentation.surfaceFormats);
		presentMode = ChooseSwapPresentMode(physicalDevice.presentation.presentModes);
		extent = physicalDevice.GetSwapExtent(surface, desiredExtent);
		preTransform = physicalDevice.GetPreTransform(surface);

		uint32_t const imageCountMinimum = GetImageCountMinimum(physicalDevice.presentation.capabilities);

		//Copy the previous data that will be cleaned up later.
		VkSwapchainKHR previousSwapchain = swapchain;
		l_vector<VulkanSwapchainImage> previousImages{images.begin(), images.end(), CTX.Temp};

		//Create the new swapchain
		{
			VkSwapchainCreateInfoKHR swapchainCI = {};
			swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCI.surface = surface;

			//Image settings
			swapchainCI.minImageCount = imageCountMinimum;
			swapchainCI.imageFormat = surfaceFormat.format;
			swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
			swapchainCI.imageExtent = extent;
			swapchainCI.imageArrayLayers = 1;
			swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			//Queue settings
			uint32_t const queueFamilyIndices[2] = {physicalDevice.queues.graphics->index, physicalDevice.queues.present->index};
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
			swapchainCI.preTransform = preTransform;

			//Compositing settings
			swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

			//Presentation settings
			swapchainCI.presentMode = presentMode;
			swapchainCI.clipped = true;

			//The old swapchain that is being replaced, which will be destroyed after the new one is created.
			swapchainCI.oldSwapchain = previousSwapchain;

			bSuccess &= vkCreateSwapchainKHR(logicalDevice, &swapchainCI, nullptr, &swapchain);
		}

		//Clean up the previous swapchain (regardless of whether the creation of the new one is a success)
		if (previousSwapchain) {
			DestroyResources(logicalDevice, previousSwapchain, previousImages);
		}

		//Get the swap chain images and create image views
		if (bSuccess) {
			uint32_t imageCount;
			vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
			VkImage* imagesRaw = CTX.Temp.Request<VkImage>(imageCount);
			vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, imagesRaw);

			VkImageViewCreateInfo imageViewCI = {};
			imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			//imageViewCI.image = swapChainImages[i];

			//Image data settings
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCI.format = surfaceFormat.format;

			//Component swizzling settings
			imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			//Image usage settings
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCI.subresourceRange.baseMipLevel = 0;
			imageViewCI.subresourceRange.levelCount = 1;
			imageViewCI.subresourceRange.baseArrayLayer = 0;
			imageViewCI.subresourceRange.layerCount = 1;

			// Get the swap chain buffers containing the image and imageview
			images.resize(imageCount);
			bool bSuccess = true;
			for (uint32_t imageIndex = 0; imageIndex < imageCount; ++imageIndex) {
				VkImage const& image = imagesRaw[imageIndex];
				VulkanSwapchainImage& data = images[imageIndex];

				//Assign the image handle
				data.image = image;

				//Create the view for this image
				imageViewCI.image = image;
				bSuccess &= vkCreateImageView(logicalDevice, &imageViewCI, nullptr, &data.view) == VK_SUCCESS;
			}
		}

		return bSuccess;
	}

	void VulkanSwapchain::Destroy(VkDevice const& logicalDevice) {
		if (swapchain) {
			DestroyResources(logicalDevice, swapchain, images);
		}
	}

	uint32_t VulkanSwapchain::GetImageCountMinimum(VkSurfaceCapabilitiesKHR const& capabilities) {
		uint32_t const maxImageCountActual = capabilities.maxImageCount > 0 ? capabilities.maxImageCount : std::numeric_limits<uint32_t>::max();
		return std::min<uint32_t>(capabilities.minImageCount + 1, maxImageCountActual);
	}

	VkSurfaceFormatKHR VulkanSwapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats) {
		for (const auto& availableSurfaceFormat : availableSurfaceFormats) {
			if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableSurfaceFormat;
			}
		}
		return availableSurfaceFormats[0];
	}

	VkPresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void VulkanSwapchain::DestroyResources(VkDevice const& device, VkSwapchainKHR const& swapchain, TArrayView<VulkanSwapchainImage const> const& images) {
		for (VulkanSwapchainImage const& image : images) {
			if (image.view) {
				vkDestroyImageView(device, image.view, nullptr);
			}
		}
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}
}
