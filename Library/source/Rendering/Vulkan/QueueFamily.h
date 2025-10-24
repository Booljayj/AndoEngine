#pragma once
#include "Engine/Flags.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Flags that describe how a queue can be used */
	enum struct EQueueFlags : uint8_t {
		Present,
		Graphics,
		Transfer,
		SparseBinding,
		Compute,
	};
	/** Flags that describe how a queue can be used */
	DEFINE_FLAGS_STRUCT(QueueFlags) {
		using TFlags::TFlags;

		/** Create a set of flags based on the raw vulkan flags information */
		static FQueueFlags Create(VkQueueFlags flags);
	};

	/** Information about a queue family on a device */
	struct QueueFamilyDescription {
		/** The unique ID of this family */
		uint32_t id = 0;
		/** The number of queues in this family */
		uint32_t size = 0;
		/** The flags that describe how the queues in this family can be used */
		FQueueFlags flags;
	};
}
