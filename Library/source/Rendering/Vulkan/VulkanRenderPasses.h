#pragma once
#include "Engine/ArrayView.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	struct VulkanRenderPasses {
		/** The main render pass used each frame */
		VkRenderPass mainRenderPass = nullptr;
		/** Clear values for each of the attachments in the main render pass */
		VkClearValue mainClearValues[1];

		inline operator bool() const { return !!mainRenderPass; }

		bool Create(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain);
		void Destroy(VulkanLogicalDevice const& logical);
	};

	struct ScopedRenderPass {
		VkCommandBuffer cachedBuffer = nullptr;

		ScopedRenderPass(VkCommandBuffer buffer, VkRenderPass pass, TArrayView<VkClearValue const> clearValues, VkFramebuffer framebuffer, VkOffset2D offset, VkExtent2D extent);
		~ScopedRenderPass();
	};
}
