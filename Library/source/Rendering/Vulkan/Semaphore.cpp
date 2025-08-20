#include "Rendering/Vulkan/Semaphore.h"

namespace Rendering {
	Semaphore::Semaphore(VkDevice device, VkSemaphoreCreateFlags flags)
		: device(device)
	{
		VkSemaphoreCreateInfo const semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.flags = flags,
		};

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS || !semaphore) {
			throw std::runtime_error{ "Failed to create semaphore" };
		}
	}

	Semaphore::~Semaphore() {
		if (device) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	}
}