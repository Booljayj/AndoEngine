#include "Rendering/Vulkan/PresentQueue.h"

namespace Rendering {
	void PresentQueue::Present(std::span<VkSemaphore const> wait_semaphores, std::span<VkSwapchainKHR const> swapchains, std::span<uint32_t const> swapchain_image_indices) const {
		VkPresentInfoKHR const info{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,

			.waitSemaphoreCount = static_cast<uint32_t>(std::size(wait_semaphores)),
			.pWaitSemaphores = wait_semaphores.data(),

			.swapchainCount = static_cast<uint32_t>(std::size(swapchains)),
			.pSwapchains = swapchains.data(),
			.pImageIndices = swapchain_image_indices.data(),

			.pResults = nullptr, //Optional
		};

		if (vkQueuePresentKHR(queue, &info) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to present to queue" };
		}
	}
}
