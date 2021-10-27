#pragma once
#include "Engine/ArrayView.h"
#include "Geometry/ScreenRect.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Information about a render pass which can be passed to various rendering methods */
	struct VulkanRenderPass {
		VkRenderPass pass;
		std::vector<VkClearValue> clearValues;
	};

	/** A scope within which rendering happens on the given render pass */
	struct ScopedRenderPass {
		VkCommandBuffer cachedBuffer = nullptr;

		ScopedRenderPass(VkCommandBuffer buffer, VulkanRenderPass const& pass, VkFramebuffer framebuffer, Geometry::ScreenRect const& position);
		~ScopedRenderPass();
	};
}
