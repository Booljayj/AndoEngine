#pragma once
#include "Engine/Core.h"
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct CommandPool {
	public:
		CommandPool(VkDevice inDevice, uint32_t queueFamilyIndex);
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
}
