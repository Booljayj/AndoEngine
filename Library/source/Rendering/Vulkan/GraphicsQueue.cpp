#include "Rendering/Vulkan/GraphicsQueue.h"

namespace Rendering {
	void GraphicsQueue::Submit(std::span<VkSemaphore const> wait_semaphores, std::span<VkPipelineStageFlags const> wait_stages, std::span<VkCommandBuffer const> command_buffers, std::span<VkSemaphore const> signal_semaphores, VkFence fence) const {
		if (wait_semaphores.size() != wait_stages.size()) throw std::runtime_error{ "wait_semaphores and wait_stages must have the same size" };

		VkSubmitInfo const info{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,

			.waitSemaphoreCount = static_cast<uint32_t>(std::size(wait_semaphores)),
			.pWaitSemaphores = wait_semaphores.data(),
			.pWaitDstStageMask = wait_stages.data(),

			.commandBufferCount = static_cast<uint32_t>(std::size(command_buffers)),
			.pCommandBuffers = command_buffers.data(),

			.signalSemaphoreCount = static_cast<uint32_t>(std::size(signal_semaphores)),
			.pSignalSemaphores = signal_semaphores.data(),
		};

		if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to submit commands to queue" };
		}
	}

	GraphicsCommandPool::GraphicsCommandPool(VkDevice device, GraphicsQueue graphics)
		: device(device)
	{
		VkCommandPoolCreateInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0, // Optional
			.queueFamilyIndex = graphics.id,
		};

		if (vkCreateCommandPool(device, &info, nullptr, &pool) != VK_SUCCESS || !pool) {
			LOG(Vulkan, Error, "Failed to create command pool");
			throw std::runtime_error{ "Failed to create command pool" };
		}
	}

	GraphicsCommandPool::~GraphicsCommandPool() {
		if (device) vkDestroyCommandPool(device, pool, nullptr);
	}

	void GraphicsCommandPool::Reset() {
		//Reset the command buffers used for this frame
		if (vkResetCommandPool(device, pool, 0) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Unable to reset command pool");
			throw std::runtime_error{ "Unable to reset command pool" };
		}
	}

	VkCommandBuffer GraphicsCommandPool::CreateBuffer(VkCommandBufferLevel level) {
		VkCommandBufferAllocateInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = pool,
			.level = level,
			.commandBufferCount = 1,
		};

		VkCommandBuffer buffer = nullptr;
		if (vkAllocateCommandBuffers(device, &info, &buffer) != VK_SUCCESS || !buffer) {
			throw std::runtime_error{ "Failed to allocate command buffer" };
		}

		return buffer;
	}

	void GraphicsCommandPool::DestroyBuffer(VkCommandBuffer buffer) {
		vkFreeCommandBuffers(device, pool, 1, &buffer);
	}

	std::vector<VkCommandBuffer> GraphicsCommandPool::CreateBuffers(uint32_t numBuffers, VkCommandBufferLevel level) {
		if (numBuffers < 1) throw std::runtime_error{ "Cannot create a nonzero number of buffers" };

		VkCommandBufferAllocateInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = pool,
			.level = level,
			.commandBufferCount = numBuffers,
		};

		std::vector<VkCommandBuffer> buffers;
		buffers.resize(numBuffers, nullptr);

		if (vkAllocateCommandBuffers(device, &info, buffers.data()) != VK_SUCCESS || !buffers[0]) {
			throw std::runtime_error{ "Failed to allocate command buffer" };
		}

		return buffers;
	}

	void GraphicsCommandPool::DestroyBuffers(std::span<VkCommandBuffer const> buffers) {
		vkFreeCommandBuffers(device, pool, buffers.size(), buffers.data());
	}

	GraphicsCommandWriter::GraphicsCommandWriter(VkCommandBuffer commands)
		: commands(commands)
	{
		VkCommandBufferBeginInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr,
		};

		if (vkBeginCommandBuffer(commands, &info) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to begin recording graphics command buffer" };
		}
	}

	GraphicsCommandWriter::GraphicsCommandWriter(VkCommandBuffer commands, CommandInheritance const& inheritance)
		: commands(commands)
	{
		VkCommandBufferInheritanceInfo inherit{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = inheritance.renderPass,
			.subpass = inheritance.subpass,
			.framebuffer = inheritance.framebuffer,
			.occlusionQueryEnable = inheritance.occlusionQueryEnable,
			.queryFlags = inheritance.queryFlags,
			.pipelineStatistics = inheritance.pipelineStatistics,
		};

		VkCommandBufferBeginInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &inherit,
		};

		if (vkBeginCommandBuffer(commands, &info) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to begin recording graphics command buffer" };
		}
	}

	GraphicsCommandWriter::~GraphicsCommandWriter() {
		if (vkEndCommandBuffer(commands) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to finish recording command buffer");
		}
	}
}
