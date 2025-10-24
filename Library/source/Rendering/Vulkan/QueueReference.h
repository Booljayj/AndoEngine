#pragma once
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Reference to a specific queue on a device */
	struct QueueReference {
		/** The unique ID of the family to which this queue belongs */
		uint32_t id = 0;
		/** The index of this queue within its family */
		uint32_t index = 0;

		inline bool operator==(QueueReference const&) const = default;
	};
}
