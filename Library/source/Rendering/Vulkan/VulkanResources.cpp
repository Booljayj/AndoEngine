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
		buffer = nullptr;
		allocation = nullptr;
		return result;
	}

	UniqueVulkanBuffer CreateBuffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage) {
		UniqueVulkanBuffer result;
		result.allocator = allocator;

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = bufferUsage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI = {};
		allocCI.usage = allocationUsage;

		vmaCreateBuffer(allocator, &bufferCI, &allocCI, &result.buffer, &result.allocation, nullptr);
		return result;
	}
}
