#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Engine/LogCommands.h"

namespace Rendering {
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
