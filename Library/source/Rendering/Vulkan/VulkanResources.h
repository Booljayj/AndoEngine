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

	/** A buffer and the memory allocation for it */
	struct VulkanBuffer {
		VkBuffer buffer = nullptr;
		VmaAllocation allocation = nullptr;

		inline operator bool() const { return buffer && allocation; }

		inline void Destroy(VmaAllocator allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			buffer = nullptr;
			allocation = nullptr;
		}
	};

	struct VulkanMeshResources {
		VulkanBuffer vertex;
		VulkanBuffer index;

		inline operator bool() const { return vertex && index; }

		inline void Destroy(VmaAllocator allocator) {
			vertex.Destroy(allocator);
			index.Destroy(allocator);
		}
	};
}
