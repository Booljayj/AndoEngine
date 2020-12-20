#include "Rendering/Vulkan/VulkanResources.h"

namespace Rendering {
	VulkanBuffer CreateBuffer(VmaAllocator allocator, size_t capacity, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage) {
		VulkanBuffer result;

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = capacity;
		bufferCI.usage = bufferUsage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI{};
		allocCI.usage = allocationUsage;

		VmaAllocationInfo info{};
		vmaCreateBuffer(allocator, &bufferCI, &allocCI, &result.buffer, &result.allocation, &info);
		result.capacity = info.size;

		return result;
	}

	VulkanMappedBuffer CreateMappedBuffer(VmaAllocator allocator, size_t capacity, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage) {
		VulkanMappedBuffer result;

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = capacity;
		bufferCI.usage = bufferUsage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI{};
		allocCI.usage = allocationUsage;
		allocCI.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo info{};
		vmaCreateBuffer(allocator, &bufferCI, &allocCI, &result.buffer, &result.allocation, &info);
		result.capacity = info.size;
		result.mapped = static_cast<char*>(info.pMappedData);

		return result;
	}

	EResourceModifyResult VulkanUniformResources::Reserve(VmaAllocator allocator, size_t newElementSize, size_t newCapacity) {
		if (newCapacity > uniforms.capacity) {
			elementSize = newElementSize;
			uniforms.Destroy(allocator);

			uniforms = CreateMappedBuffer(allocator, newCapacity, VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
			if (!uniforms) return EResourceModifyResult::Error;
			else return EResourceModifyResult::Modified;

		} else if (newElementSize != elementSize) {
			elementSize = newElementSize;
			return EResourceModifyResult::Modified;
		}

		return EResourceModifyResult::Unmodified;
	}
}
