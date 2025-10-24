#pragma once
#include "Engine/Core.h"
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/QueueReference.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Non-owning handle to a specific queue that was created on a device and which can be used for transfer operations */
	struct TransferQueue : public QueueReference {
		TransferQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Submit commands to this queue */
		void Submit(std::span<VkSemaphore const> wait_semaphores, std::span<VkPipelineStageFlags const> wait_stages, std::span<VkCommandBuffer const> command_buffers, std::span<VkSemaphore const> signal_semaphores, VkFence fence) const;
		void Submit(std::span<VkCommandBuffer const> command_buffers, VkFence fence) const;

	protected:
		VkQueue queue = nullptr;
	};

	/** A pool that can be used to allocate command buffers that are appropriate for transfer commands */
	struct TransferCommandPool {
	public:
		TransferCommandPool(VkDevice device, TransferQueue transfer);
		TransferCommandPool(TransferCommandPool const&) = delete;
		TransferCommandPool(TransferCommandPool&&) noexcept = default;
		~TransferCommandPool();

		inline operator VkCommandPool() const { return pool; }

		/** Reset the memory in the pool so it can be used by new command buffers */
		void Reset();

		VkCommandBuffer CreateBuffer(VkCommandBufferLevel level);
		void DestroyBuffer(VkCommandBuffer buffer);

		std::vector<VkCommandBuffer> CreateBuffers(uint32_t numBuffers, VkCommandBufferLevel level);
		void DestroyBuffers(std::span<VkCommandBuffer const> buffers);

	private:
		MoveOnly<VkDevice> device;
		VkCommandPool pool = nullptr;
	};

	struct TransferCommandWriter {
		template<typename T>
		using Span = std::span<T const>;

		explicit TransferCommandWriter(VkCommandBuffer commands);
		TransferCommandWriter(TransferCommandWriter const&) = delete;
		TransferCommandWriter(TransferCommandWriter&&) = delete;
		~TransferCommandWriter();

		inline void Copy(VkBuffer source, VkBuffer destination, Span<VkBufferCopy> regions) {
			vkCmdCopyBuffer(commands, source, destination, regions.size(), regions.data());
		}

	private:
		VkCommandBuffer commands;
	};
}
