#include "Rendering/Vulkan/ComputeQueue.h"

namespace Rendering {
	void ComputeQueue::Submit(VkSubmitInfo const& info, VkFence fence) const {
		if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to submit commands to queue" };
		}
	}
}
