#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Engine/LogCommands.h"

namespace Rendering {
	ScopedRenderPass::ScopedRenderPass(VkCommandBuffer buffer, VkRenderPass pass, TArrayView<VkClearValue> clearValues, VkFramebuffer framebuffer, Geometry::ScreenRect const& position) {
		cachedBuffer = buffer;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = VkOffset2D{ position.offset.x, position.offset.y };
		renderPassInfo.renderArea.extent = VkExtent2D{ position.extent.x, position.extent.y };

		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.begin();

		vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	ScopedRenderPass::~ScopedRenderPass() {
		vkCmdEndRenderPass(cachedBuffer);
	}
}
