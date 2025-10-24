#pragma once
#include "Engine/Core.h"
#include "Rendering/Vulkan/QueueReference.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Non-owning handle to a specific queue that was created on a device and which can be used for compute operations */
	struct ComputeQueue : public QueueReference {
		ComputeQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Submit commands on this queue */
		void Submit(VkSubmitInfo const& info, VkFence fence) const;

	protected:
		VkQueue queue = nullptr;
	};
}
