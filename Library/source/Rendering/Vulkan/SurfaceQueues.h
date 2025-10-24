#pragma once
#include "Rendering/Vulkan/GraphicsQueue.h"
#include "Rendering/Vulkan/PresentQueue.h"

namespace Rendering {
	/** A collection of queues that can be used for surface-related operations on a device */
	struct SurfaceQueues {
		/** The queue that can be used for present operations */
		PresentQueue present;
		/** The queue that can be used for general graphics operations */
		GraphicsQueue graphics;

		/** A collection of references to usable surface queues. Can be used to create or find those qeues. */
		struct References {
			/** A reference to the queue that can be used for present operations */
			QueueReference present;
			/** A reference to the queue that can be used for general graphics operations */
			QueueReference graphics;
		};
	};
}
