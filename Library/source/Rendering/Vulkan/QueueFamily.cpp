#include "Rendering/Vulkan/QueueFamily.h"

namespace Rendering {
	FQueueFlags FQueueFlags::Create(VkQueueFlags flags) {
		FQueueFlags result;
		if ((flags & VK_QUEUE_GRAPHICS_BIT) > 0) result += EQueueFlags::Graphics;
		if ((flags & VK_QUEUE_TRANSFER_BIT) > 0) result += EQueueFlags::Transfer;
		if ((flags & VK_QUEUE_SPARSE_BINDING_BIT) > 0) result += EQueueFlags::SparseBinding;
		if ((flags & VK_QUEUE_COMPUTE_BIT) > 0) result += EQueueFlags::Compute;
		return result;
	}
}
