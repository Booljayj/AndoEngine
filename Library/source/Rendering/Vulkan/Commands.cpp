#include "Rendering/Vulkan/Commands.h"

namespace Rendering {
	CommandPool::CommandPool(VkDevice device, uint32_t id)
		: device(device)
	{
		VkCommandPoolCreateInfo const poolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = 0, // Optional
			.queueFamilyIndex = id,
		};

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS || !pool) {
			LOG(Vulkan, Error, "Failed to create command pool");
			throw std::runtime_error{ "Failed to create command pool" };
		}
	}

	CommandPool::~CommandPool() {
		if (device) vkDestroyCommandPool(device, pool, nullptr);
	}

	void CommandPool::Reset() {
		//Reset the command buffers used for this frame
		if (vkResetCommandPool(device, pool, 0) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Unable to reset command pool");
			throw std::runtime_error{ "Unable to reset command pool" };
		}
	}

	VkCommandBuffer CommandPool::CreateBuffer(VkCommandBufferLevel level) {
		VkCommandBufferAllocateInfo const bufferAllocationInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = pool,
			.level = level,
			.commandBufferCount = 1,
		};

		VkCommandBuffer buffer = nullptr;
		if (vkAllocateCommandBuffers(device, &bufferAllocationInfo, &buffer) != VK_SUCCESS || !buffer) {
			throw std::runtime_error{ "Failed to allocate command buffer" };
		}

		return buffer;
	}

	void CommandPool::DestroyBuffer(VkCommandBuffer buffer) {
		vkFreeCommandBuffers(device, pool, 1, &buffer);
	}

	std::vector<VkCommandBuffer> CommandPool::CreateBuffers(uint32_t numBuffers, VkCommandBufferLevel level) {
		if (numBuffers < 1) throw std::runtime_error{ "Cannot create a nonzero number of buffers" };

		VkCommandBufferAllocateInfo const bufferAllocationInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = pool,
			.level = level,
			.commandBufferCount = numBuffers,
		};
		
		std::vector<VkCommandBuffer> buffers;
		buffers.resize(numBuffers, nullptr);

		if (vkAllocateCommandBuffers(device, &bufferAllocationInfo, buffers.data()) != VK_SUCCESS || !buffers[0]) {
			throw std::runtime_error{ "Failed to allocate command buffer" };
		}

		return buffers;
	}

	void CommandPool::DestroyBuffers(std::span<VkCommandBuffer const> buffers) {
		vkFreeCommandBuffers(device, pool, buffers.size(), buffers.data());
	}

	ScopedCommands::ScopedCommands(VkCommandBuffer buffer, VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo const* inheritance)
		: buffer(buffer)
	{
		VkCommandBufferBeginInfo const beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = flags,
			.pInheritanceInfo = inheritance,
		};
		
		if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to begin recording command buffer" };
		}
	}

	ScopedCommands::~ScopedCommands() {
		if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to finish recording command buffer");
		}
	}

	GraphicsCommandWriter::GraphicsCommandWriter(GraphicsCommandBuffer graphics_buffer, VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo const* inheritance)
		: buffer(static_cast<VkCommandBuffer>(graphics_buffer))
	{
		VkCommandBufferBeginInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = flags,
			.pInheritanceInfo = inheritance,
		};

		if (vkBeginCommandBuffer(buffer, &info) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to begin recording command buffer" };
		}
	}

	GraphicsCommandWriter::~GraphicsCommandWriter() {
		if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to finish recording command buffer");
		}
	}
}
