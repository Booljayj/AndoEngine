#include "Rendering/Vulkan/Fence.h"

namespace Rendering {
	Fence::Fence(VkDevice device)
		: device(device)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS || !fence) {
			throw std::runtime_error{ "Failed to create fence" };
		}
	}

	Fence::Fence(Fence&& other) noexcept
		: device(other.device), fence(other.fence)
	{
		other.device = nullptr;
	}

	Fence::~Fence() {
		if (device) {
			vkWaitForFences(device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
			vkDestroyFence(device, fence, nullptr);
		}
	}

	bool Fence::WaitUntilSignalled(std::chrono::nanoseconds timeout) const {
		return vkWaitForFences(device, 1, &fence, VK_TRUE, timeout.count()) == VK_SUCCESS;
	}

	bool Fence::IsSignalled() const {
		return vkGetFenceStatus(device, fence) == VK_SUCCESS;
	}

	void Fence::Reset() const {
		if (vkResetFences(device, 1, &fence) != VK_SUCCESS) throw std::runtime_error{ "Unable to reset fence" };
	}
}
