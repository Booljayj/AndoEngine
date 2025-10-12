#pragma once
#include "Engine/Core.h"
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/Queues.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct CommandPool {
	public:
		CommandPool(VkDevice device, GraphicsQueue graphics) : CommandPool(device, graphics.id) {}
		CommandPool(VkDevice device, TransferQueue transfer) : CommandPool(device, transfer.id) {}
		CommandPool(VkDevice device, ComputeQueue compute) : CommandPool(device, compute.id) {}

		CommandPool(CommandPool const&) = delete;
		CommandPool(CommandPool&&) noexcept = default;
		~CommandPool();

		inline operator VkCommandPool() const { return pool; }

		/** Reset the memory in the pool so it can be used by new command buffers */
		void Reset();

		VkCommandBuffer CreateBuffer(VkCommandBufferLevel level);
		void DestroyBuffer(VkCommandBuffer buffer);

		std::vector<VkCommandBuffer> CreateBuffers(uint32_t numBuffers, VkCommandBufferLevel level);
		void DestroyBuffers(std::span<VkCommandBuffer const> buffers);

	private:
		CommandPool(VkDevice device, uint32_t id);

		MoveOnly<VkDevice> device;
		VkCommandPool pool = nullptr;
	};

	/** A scope within which commands can be written to the provided buffer */
	struct ScopedCommands {
		ScopedCommands(VkCommandBuffer buffer, VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo const* inheritance);
		ScopedCommands(ScopedCommands const&) = delete;
		ScopedCommands(ScopedCommands&&) = delete;
		~ScopedCommands();

	private:
		VkCommandBuffer buffer;
	};

	struct GraphicsCommandBuffer {
		explicit operator VkCommandBuffer() const { return buffer; }

	private:
		VkCommandBuffer buffer;
	};

	struct GraphicsCommandWriter {
		GraphicsCommandWriter(GraphicsCommandBuffer graphics_buffer, VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo const* inheritance);
		GraphicsCommandWriter(ScopedCommands const&) = delete;
		GraphicsCommandWriter(ScopedCommands&&) = delete;
		~GraphicsCommandWriter();



	private:
		VkCommandBuffer buffer;
	};
}
