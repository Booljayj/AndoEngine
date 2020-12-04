#pragma once
#include "Rendering/Vulkan/VulkanCommon.h"

namespace Rendering {
	/** Stores resources related to a graphics pipeline */
	struct VulkanPipelineResources {
		VkPipelineLayout layout = nullptr;
		VkPipeline pipeline = nullptr;

		inline operator bool() const { return pipeline && layout; }

		inline void Destroy(VkDevice device) {
			if (pipeline) vkDestroyPipeline(device, pipeline, nullptr);
			if (layout) vkDestroyPipelineLayout(device, layout, nullptr);
			pipeline = nullptr;
			layout = nullptr;
		}
	};
}
