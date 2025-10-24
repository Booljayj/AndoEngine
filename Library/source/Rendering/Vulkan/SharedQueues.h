#pragma once
#include "Engine/Array.h"
#include "Rendering/Vulkan/ComputeQueue.h"
#include "Rendering/Vulkan/TransferQueue.h"

namespace Rendering {
	/** A collection of queues that can be used for general shared purposes on a device */
	struct SharedQueues {
		/** The queues that can be used for transfer operations */
		std::vector<TransferQueue> transfers;
		/** The queues that can be used for compute operations */
		std::vector<ComputeQueue> computes;

		struct References {
			/** References to the queues that can be used for transfer operations */
			std::vector<QueueReference> transfers;
			/** References to the queues that can be used for compute operations */
			std::vector<QueueReference> computes;
		};
	};
}
