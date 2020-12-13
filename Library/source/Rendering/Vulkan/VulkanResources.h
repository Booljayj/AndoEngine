#pragma once
#include "Rendering/Vulkan/Vulkan.h"

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
		inline operator VkBuffer() const { return buffer; }
		inline operator VmaAllocation() const { return allocation; }
		inline void Destroy(VmaAllocator allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			buffer = nullptr;
			allocation = nullptr;
		}
	};
	/** Holds a vulkan buffer that will be destroyed at the end of the scope unless released */
	struct UniqueVulkanBuffer {
		VkBuffer buffer = nullptr;
		VmaAllocation allocation = nullptr;
		VmaAllocator allocator = nullptr;

		UniqueVulkanBuffer() = default;
		UniqueVulkanBuffer(UniqueVulkanBuffer&& other);
		~UniqueVulkanBuffer();

		operator bool() const { return buffer && allocation; }
		VulkanBuffer Release();
	};

	/** Create a buffer */
	UniqueVulkanBuffer CreateBuffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage);

	/** Stores resources related to a mesh */
	struct VulkanMeshResources {
		VulkanBuffer buffer;

		struct {
			VkDeviceSize vertex;
			VkDeviceSize index;
		} offset;

		struct {
			uint32_t vertices;
			uint32_t indices;
		} size;

		inline operator bool() const { return !!buffer; }
		inline void Destroy(VmaAllocator allocator) {
			buffer.Destroy(allocator);
		}
	};

	/** The results of creating the resources for a new mesh */
	struct VulkanMeshCreationResults {
		VkCommandBuffer commands;
		VulkanBuffer staging;
	};
}
