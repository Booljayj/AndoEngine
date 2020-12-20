#pragma once
#include "Engine/ArrayView.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	/** Holds all the values needed by a single render pass (plus any subpasses) with a certain number of attachments */
	template<size_t NumAttachments>
	struct TRenderPassInfo {
		static_assert(NumAttachments > 0, "NumAttachments must be greater than 0");

		/** The internal render pass object */
		VkRenderPass pass = nullptr;
		/** The clear values for each attachment in the render pass */
		std::array<VkClearValue, NumAttachments> clearValues;
		/** The frambuffers for each swapchain image to use with this render pass */
		std::vector<VkFramebuffer> framebuffers;

		void Destroy(VulkanLogicalDevice const& logical) {
			if (pass) vkDestroyRenderPass(logical.device, pass, nullptr);
			for (VkFramebuffer framebuffer : framebuffers) {
				if (framebuffer) vkDestroyFramebuffer(logical.device, framebuffer, nullptr);
			}
			framebuffers.clear();
			pass = nullptr;
		}
	};

	struct ScopedRenderPass {
		VkCommandBuffer cachedBuffer = nullptr;

		template<size_t NumAttachments>
		ScopedRenderPass(VkCommandBuffer buffer, TRenderPassInfo<NumAttachments> info, size_t index, VkOffset2D offset, VkExtent2D extent)
		: ScopedRenderPass(buffer, info.pass, MakeView(info.clearValues), info.framebuffers[index], offset, extent)
		{}

		ScopedRenderPass(VkCommandBuffer buffer, VkRenderPass pass, TArrayView<VkClearValue> clearValues, VkFramebuffer framebuffer, VkOffset2D offset, VkExtent2D extent);
		~ScopedRenderPass();
	};
}
