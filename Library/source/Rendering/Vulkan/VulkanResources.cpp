#include "Rendering/Vulkan/VulkanResources.h"

namespace Rendering {
	UniqueVulkanBuffer::UniqueVulkanBuffer(UniqueVulkanBuffer&& other) {
		std::swap(buffer, other.buffer);
		std::swap(allocation, other.allocation);
		std::swap(allocator, other.allocator);
	}

	UniqueVulkanBuffer::~UniqueVulkanBuffer() {
		vmaDestroyBuffer(allocator, buffer, allocation);
	}

	VulkanBuffer UniqueVulkanBuffer::Release() {
		VulkanBuffer result;
		result.buffer = buffer;
		result.allocation = allocation;
		result.capacity = capacity;
		buffer = nullptr;
		allocation = nullptr;
		return result;
	}

	UniqueVulkanBuffer CreateBuffer(VmaAllocator allocator, size_t capacity, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage) {
		UniqueVulkanBuffer result;
		result.allocator = allocator;

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
			if (!uniforms) {
				return EResourceModifyResult::Error;
			}
			return EResourceModifyResult::Modified;

		} else if (newElementSize != elementSize) {
			elementSize = newElementSize;
			return EResourceModifyResult::Modified;
		}

		return EResourceModifyResult::Unmodified;
	}
}
