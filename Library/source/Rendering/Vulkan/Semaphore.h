#pragma once
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct Semaphore {
		Semaphore(VkDevice device, VkSemaphoreCreateFlags flags);
		Semaphore(Semaphore const&) = delete;
		Semaphore(Semaphore&&) noexcept = default;
		~Semaphore();

		inline operator VkSemaphore() const { return semaphore; }

	private:
		MoveOnly<VkDevice> device;
		VkSemaphore semaphore = nullptr;
	};
}
