#pragma once
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"

namespace Rendering {
	/** Keeps track of Vulkan command buffers that are used for rendering commands */
	struct VulkanCommands {
		using RenderCommandMethod = void(*)(VkCommandBuffer);

		/** The pool that buffers are allocated from */
		VkCommandPool pool;
		/** The current list of command buffers that contain rendering commands */
		std::vector<VkCommandBuffer> buffers;

		operator bool() const { return !!pool; }

		static VulkanCommands Create(CTX_ARG, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical);
		void Destroy(VulkanLogicalDevice const& logical);

		/** Reallocate command buffers for the swapchain. The new command buffers are ready to record commands */
		bool ReallocateCommandBuffers(CTX_ARG, VulkanLogicalDevice const& logical, size_t numBuffers);
	};

	//Helper template used to record commands in a buffer correctly. Commands are recorded by the provided "record" functor.
	template<typename FunctorType>
	bool RecordCommandBuffer(CTX_ARG, VkCommandBuffer buffer, FunctorType&& record) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Failed to begin recording command buffer %i", index);
			return false;
		}

		record(buffer);

		if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Failed to finish recording command buffer %i", index);
			return false;
		}

		return true;
	}
}
