#include "Rendering/Vulkan/VulkanResources.h"

namespace Rendering {
	VulkanBuffer CreateBuffer(VulkanLogicalDevice const& logical, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) {
		VulkanBuffer result;

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = bufferUsage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI = {};
		allocCI.usage = memoryUsage;

		vmaCreateBuffer(logical.allocator, &bufferCI, &allocCI, &result.buffer, &result.allocation, nullptr);
		return result;
	}

	VulkanBufferCreationResults CreateBufferWithData(VulkanLogicalDevice const& logical, void const* source, size_t size, VkBufferUsageFlags usage, VkCommandBuffer commands) {
		VulkanBufferCreationResults result;
		if (size == 0) return result;

		//Create the staging buffer
		result.staging = CreateBuffer(logical, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if (!result.staging) return result;

		//Create the GPU buffer
		result.gpu = CreateBuffer(logical, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		if (!result.gpu) {
			result.staging.Destroy(logical.allocator);
			return result;
		}

		//Fill the staging buffer with the source data
		void* data;
		if (vmaMapMemory(logical.allocator, result.staging.allocation, &data) != VK_SUCCESS) {
			result.staging.Destroy(logical.allocator);
			result.gpu.Destroy(logical.allocator);
			return result;
		}
        memcpy(data, source, size);
		vmaUnmapMemory(logical.allocator, result.staging.allocation);

		//Record the command to transfer from the staging buffer to the gpu buffer
		VkBufferCopy copy{};
		copy.srcOffset = 0; // Optional
		copy.dstOffset = 0; // Optional
		copy.size = size;
		vkCmdCopyBuffer(commands, result.staging.buffer, result.gpu.buffer, 1, &copy);

		return result;
	}
}
