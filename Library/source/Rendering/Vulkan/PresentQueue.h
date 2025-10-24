#pragma once
#include "Engine/Core.h"
#include "Rendering/Vulkan/QueueReference.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Non-owning handle to a specific queue that was created on a device and which can be used for present operations */
	struct PresentQueue : public QueueReference {
		PresentQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Present on this queue */
		void Present(std::span<VkSemaphore const> wait_semaphores, std::span<VkSwapchainKHR const> swapchains, std::span<uint32_t const> swapchain_image_indices) const;

	protected:
		VkQueue queue = nullptr;
	};
}
