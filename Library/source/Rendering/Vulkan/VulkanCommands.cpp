#include "Rendering/Vulkan/VulkanCommands.h"
#include "Engine/LogCommands.h"

namespace Rendering {
	VulkanCommands VulkanCommands::Create(CTX_ARG, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical) {
		VulkanCommands result;

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = physical.queues.graphics.value().index;
		poolInfo.flags = 0; // Optional

		if (vkCreateCommandPool(logical.device, &poolInfo, nullptr, &result.pool) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create command pool");
		}
		return result;
	}

	void VulkanCommands::Destroy(VulkanLogicalDevice const& logical) {
		vkDestroyCommandPool(logical.device, pool, nullptr);
		pool = nullptr;
		//Buffers do not need to be explicitly deallocated if we're destroying the entire command pool.
		buffers.clear();
	}


	bool VulkanCommands::ReallocateCommandBuffers(CTX_ARG, VulkanLogicalDevice const& logical, size_t numBuffers) {
		//Free any existing allocated command buffers
		if (buffers.size() > 0) {
			vkFreeCommandBuffers(logical.device, pool, buffers.size(), buffers.data());
			buffers.clear();
		}

		//Allocate all of the command buffers and store the new references to the buffers
    	buffers.resize(numBuffers);

		VkCommandBufferAllocateInfo bufferAllocationInfo{};
		bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocationInfo.commandPool = pool;
		bufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAllocationInfo.commandBufferCount = (uint32_t) buffers.size();

		if (vkAllocateCommandBuffers(logical.device, &bufferAllocationInfo, buffers.data()) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to allocate command buffers from the command pool");
			return false;
		}

		return true;
	}
}
