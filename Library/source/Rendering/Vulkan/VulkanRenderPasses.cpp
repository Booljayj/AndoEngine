#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"

namespace Rendering {
	bool VulkanRenderPasses::Create(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain) {
		mainClearValues[0].color.float32[3] = 1.0f;

		//Descriptions of attachments
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapchain.surfaceFormat.format;
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

		assert(!mainRenderPass);
		if (vkCreateRenderPass(logical.device, &renderPassInfo, nullptr, &mainRenderPass) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create main render pass");
			return false;
		}

		return true;
	}

	void VulkanRenderPasses::Destroy(VulkanLogicalDevice const& logical) {
		if (mainRenderPass) {
			vkDestroyRenderPass(logical.device, mainRenderPass, nullptr);
			mainRenderPass = nullptr;
		}
	}

	ScopedRenderPass::ScopedRenderPass(VkCommandBuffer buffer, VkRenderPass pass, TArrayView<VkClearValue const> clearValues, VkFramebuffer framebuffer, VkOffset2D offset, VkExtent2D extent) {
		cachedBuffer = buffer;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = offset;
		renderPassInfo.renderArea.extent = extent;

		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.begin();

		vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	ScopedRenderPass::~ScopedRenderPass() {
		vkCmdEndRenderPass(cachedBuffer);
	}
}
