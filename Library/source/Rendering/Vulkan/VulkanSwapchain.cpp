#include "Engine/LinearContainers.h"
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

			return vkCreateSwapchainKHR(logical.device, &swapchainCI, nullptr, &result.swapchain) == VK_SUCCESS;
		}

		bool CreateImageInfo(CTX_ARG, VulkanSwapchain& result, VulkanLogicalDevice const& logical) {
			uint32_t numImages;
			vkGetSwapchainImagesKHR(logical.device, result.swapchain, &numImages, nullptr);
			VkImage* raw = CTX.temp.Request<VkImage>(numImages);
			vkGetSwapchainImagesKHR(logical.device, result.swapchain, &numImages, raw);

			VkImageViewCreateInfo imageViewCI = {};
			imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

			//Image data settings
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCI.format = result.surfaceFormat.format;

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
			result.images.resize(numImages);
			bool bSuccess = true;
			for (uint32_t imageIndex = 0; imageIndex < numImages; ++imageIndex) {
				VulkanSwapchainImageInfo& image = result.images[imageIndex];
				//Assign the image handle
				image.raw = raw[imageIndex];

				//Create the view for this image
				imageViewCI.image = image.raw;
				bSuccess &= vkCreateImageView(logical.device, &imageViewCI, nullptr, &image.view) == VK_SUCCESS;
			}

			return bSuccess;
		}

		bool CreateRenderPasses(CTX_ARG, VulkanSwapchain& result, const VulkanLogicalDevice& logical) {
			//Descriptions of attachments
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = result.surfaceFormat.format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			//References to attachments
			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			//The color subpass, which uses the color attachment
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			//Subpass dependencies
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			//Information to create the full render pass, with all relevant attachments and subpasses.
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			return vkCreateRenderPass(logical.device, &renderPassInfo, nullptr, &result.renderPass) == VK_SUCCESS;
		}
	}

	bool CreateFramebuffers(CTX_ARG, VulkanSwapchain& result, const VulkanLogicalDevice& logical) {
		size_t const numFramebuffers = result.images.size();

		for (size_t index = 0; index < numFramebuffers; ++index) {
			VkImageView attachments[1] = {result.images[index].view};

			VkFramebufferCreateInfo framebufferCI = {};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = result.renderPass;
			framebufferCI.attachmentCount = 1;
			framebufferCI.pAttachments = attachments;
			framebufferCI.width = result.extent.width;
			framebufferCI.height = result.extent.height;
			framebufferCI.layers = 1;

			if (vkCreateFramebuffer(logical.device, &framebufferCI, nullptr, &result.images[index].framebuffer) != VK_SUCCESS) {
				return false;
			}
		}

		return true;
	}

	VulkanSwapchain VulkanSwapchain::Create(CTX_ARG, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical) {
		using namespace VulkanSwapchainUtilities;
		TEMP_ALLOCATOR_MARK();

		vkDeviceWaitIdle(logical.device);

		VulkanSwapchain result;
		SetupProperties(CTX, result, extent, surface, physical);

		bool const bSuccess =
			CreateSwapchain(CTX, result, nullptr, surface, physical, logical) &&
			CreateImageInfo(CTX, result, logical) &&
			CreateRenderPasses(CTX, result, logical) &&
			CreateFramebuffers(CTX, result, logical);

		//If we only partially created everything, destroy what was created.
		if (!bSuccess) result.Destroy(logical);

		return result;
	}

	VulkanSwapchain VulkanSwapchain::Recreate(CTX_ARG, VulkanSwapchain& previous, VkExtent2D const& extent, VkSurfaceKHR const& surface, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical) {
		using namespace VulkanSwapchainUtilities;
		TEMP_ALLOCATOR_MARK();

		vkDeviceWaitIdle(logical.device);

		VulkanSwapchain result;
		SetupProperties(CTX, result, extent, surface, physical);

		bool const bCreatedSwapchain = CreateSwapchain(CTX, result, previous.swapchain, surface, physical, logical);

		//Clean up the previous swapchain (regardless of whether the creation of the new one is a success)
		if (previous) {
			previous.Destroy(logical);
		}

		//During a recreate, if we didn't even start creating the new swapchain, we can stop here.
		if (!bCreatedSwapchain) return result;

		//Get the swap chain images and create image views
		bool const bSuccess = CreateImageInfo(CTX, result, logical) &&
			CreateRenderPasses(CTX, result, logical) &&
			CreateFramebuffers(CTX, result, logical);

		//If we only partially created everything, destroy what was created.
		if (!bSuccess) result.Destroy(logical);

		return result;
	}

	void VulkanSwapchain::Destroy(VulkanLogicalDevice const& logical) {
		//Destroy the framebuffers
		for (VulkanSwapchainImageInfo const& image : images) {
			if (image.framebuffer) {
				vkDestroyFramebuffer(logical.device, image.framebuffer, nullptr);
			}
		}

		//Destroy the render pass
		if (renderPass) {
			vkDestroyRenderPass(logical.device, renderPass, nullptr);
			renderPass = nullptr;
		}

		//Destroy the image views
		for (VulkanSwapchainImageInfo const& image : images) {
			if (image.view) {
				vkDestroyImageView(logical.device, image.view, nullptr);
			}
		}

		//We have cleaned up all image-related data, so remove the information
		images.clear();

		//Destroy the swapchain
		if (swapchain) {
			vkDestroySwapchainKHR(logical.device, swapchain, nullptr);
			swapchain = nullptr;
		}
	}
}
