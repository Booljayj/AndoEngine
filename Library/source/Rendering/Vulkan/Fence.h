#pragma once
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct Fence {
		Fence(VkDevice device);
		Fence(Fence const&) = delete;
		Fence(Fence&&) noexcept = default;
		~Fence();

		inline operator VkFence() const { return fence; }

		/**
		 * Block the calling thread until the fence is signalled by the current operation.
		 * Returns true if the fence was signalled, false if an error occurred or the timeout was reached.
		 */
		bool WaitUntilSignalled(std::chrono::nanoseconds timeout) const;

		/** Returns true if the fence is currently signalled */
		bool IsSignalled() const;

		/** Reset this fence back to an unsignalled state */
		void Reset() const;

	private:
		stdext::move_only<VkDevice> device;
		VkFence fence = nullptr;
	};
}
