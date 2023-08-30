#include "Rendering/Vulkan/VulkanCommandBuffers.h"

namespace Rendering {
	CommandPool::CommandPool(VkDevice inDevice, uint32_t queueFamilyIndex)
		: device(inDevice)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = 0; // Optional

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS || !pool) {
			LOG(Vulkan, Error, "Failed to create command pool");
			throw std::runtime_error{ "Failed to create command pool" };
		}
	}

	CommandPool::CommandPool(CommandPool&& other) noexcept {
		std::swap(device, other.device);
		std::swap(pool, other.pool);
	}

	CommandPool::~CommandPool() {
		if (device) {
			vkDestroyCommandPool(device, pool, nullptr);
			device = nullptr;
		}
	}

	void CommandPool::Reset() {
		//Reset the command buffers used for this frame
		if (vkResetCommandPool(device, pool, 0) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Unable to reset command pool");
			throw std::runtime_error{ "Unable to reset command pool" };
		}
	}

	VkCommandBuffer CommandPool::CreateBuffer(VkCommandBufferLevel level) {
		VkCommandBufferAllocateInfo bufferAllocationInfo{};
		bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocationInfo.commandPool = pool;
		bufferAllocationInfo.level = level;
		bufferAllocationInfo.commandBufferCount = 1;

		VkCommandBuffer buffer = nullptr;
		if (vkAllocateCommandBuffers(device, &bufferAllocationInfo, &buffer) != VK_SUCCESS || !buffer) {
			throw std::runtime_error{ "Failed to allocate command buffer" };
		}

		return buffer;
	}

	void CommandPool::DestroyBuffer(VkCommandBuffer buffer) {
		vkFreeCommandBuffers(device, pool, 1, &buffer);
	}

	std::vector<VkCommandBuffer> CommandPool::CreateBuffers(size_t numBuffers, VkCommandBufferLevel level) {
		if (numBuffers < 1) throw std::runtime_error{ "Cannot create a nonzero number of buffers" };

		VkCommandBufferAllocateInfo bufferAllocationInfo{};
		bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocationInfo.commandPool = pool;
		bufferAllocationInfo.level = level;
		bufferAllocationInfo.commandBufferCount = numBuffers;

		std::vector<VkCommandBuffer> buffers;
		buffers.resize(numBuffers, nullptr);

		if (vkAllocateCommandBuffers(device, &bufferAllocationInfo, buffers.data()) != VK_SUCCESS || !buffers[0]) {
			throw std::runtime_error{ "Failed to allocate command buffer" };
		}

		return buffers;
	}

	void CommandPool::DestroyBuffers(TArrayView<VkCommandBuffer const> buffers) {
		vkFreeCommandBuffers(device, pool, buffers.size(), buffers.begin());
	}

	ScopedCommands::ScopedCommands(VkCommandBuffer inBuffer, VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo const* inheritance)
		: buffer(inBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flags;
		beginInfo.pInheritanceInfo = inheritance;

		if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to begin recording command buffer" };
		}
	}

	ScopedCommands::~ScopedCommands() {
		if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to finish recording command buffer");
		}
	}
}
