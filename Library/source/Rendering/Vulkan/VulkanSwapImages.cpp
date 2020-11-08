#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanSwapImages.h"

namespace Rendering {
	bool VulkanSwapImages::Create(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain& swapchain, VulkanRenderPasses const& passes) {
		TEMP_ALLOCATOR_MARK();

		//Get the raw images from the swapchain
		uint32_t numImages;
		vkGetSwapchainImagesKHR(logical.device, swapchain.swapchain, &numImages, nullptr);
		VkImage* rawImages = CTX.temp.Request<VkImage>(numImages);
		vkGetSwapchainImagesKHR(logical.device, swapchain.swapchain, &numImages, rawImages);

		if (numImages == 0) {
			LOG(Vulkan, Error, "Swapchain has no images to retrieve");
			return false;
		}

		internal.resize(numImages);
		for (uint32_t index = 0; index < numImages; ++index) {
			SwapImage& image = internal[index];

			//Create image view
			VkImageViewCreateInfo imageViewCI = {};
			imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCI.image = rawImages[index];
			//Image data settings
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCI.format = swapchain.surfaceFormat.format;
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

			assert(!image.view);
			if (vkCreateImageView(logical.device, &imageViewCI, nullptr, &image.view) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create image view %i", index);
				return false;
			}

			//Create the framebuffer
			VkFramebufferCreateInfo framebufferCI = {};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = passes.mainRenderPass;
			//Attachments
			VkImageView attachments[1] = {image.view};
			framebufferCI.attachmentCount = 1;
			framebufferCI.pAttachments = attachments;
			//Sizes and layers
			framebufferCI.width = swapchain.extent.width;
			framebufferCI.height = swapchain.extent.height;
			framebufferCI.layers = 1;

			assert(!image.framebuffer);
			if (vkCreateFramebuffer(logical.device, &framebufferCI, nullptr, &image.framebuffer) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create frambuffer %i", index);
				return false;
			}
		}

		return true;
	}

	void VulkanSwapImages::Destroy(VulkanLogicalDevice const& logical) {
		for (SwapImage const& image : internal) {
			if (image.framebuffer) vkDestroyFramebuffer(logical.device, image.framebuffer, nullptr);
			if (image.view) vkDestroyImageView(logical.device, image.view, nullptr);
		}
		internal.clear();
	}
}
