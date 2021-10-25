#pragma once
#include "Engine/ArrayView.h"
#include "Geometry/ScreenRect.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** A scope within which rendering happens on the given render pass */
	struct ScopedRenderPass {
		VkCommandBuffer cachedBuffer = nullptr;

		ScopedRenderPass(VkCommandBuffer buffer, VkRenderPass pass, TArrayView<VkClearValue> clearValues, VkFramebuffer framebuffer, Geometry::ScreenRect const& position);
		~ScopedRenderPass();
	};
}
