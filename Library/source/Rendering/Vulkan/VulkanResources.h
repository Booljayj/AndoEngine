#pragma once
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"

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

	/** The resulting buffers from uploading data to the GPU */
	struct VulkanBufferCreationResults {
		VulkanBuffer staging;
		VulkanBuffer gpu;

		inline operator bool() const { return staging && gpu; }
		inline void Destroy(VmaAllocator allocator) {
			staging.Destroy(allocator);
			gpu.Destroy(allocator);
		}
	};

	/** Create a buffer */
	VulkanBuffer CreateBuffer(VulkanLogicalDevice const& logical, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
	/** Transfer the source data to the GPU and return the resulting buffers. The transfer process is recorded to the command buffer. */
	VulkanBufferCreationResults CreateBufferWithData(VulkanLogicalDevice const& logical, void const* source, size_t size, VkBufferUsageFlags usage, VkCommandBuffer commands);

	/** Stores resources related to a mesh */
	struct VulkanMeshResources {
		VulkanBuffer vertex;
		VulkanBuffer index;
		uint32_t numIndices;

		inline operator bool() const { return vertex; }
		inline void Destroy(VmaAllocator allocator) {
			vertex.Destroy(allocator);
			index.Destroy(allocator);
		}
	};

	/** The results of creating the resources for a new mesh */
	struct VulkanMeshCreationResults {
		VkCommandBuffer commands;
		struct {
			VulkanBuffer vertex;
			VulkanBuffer index;
		} staging;
	};
}
