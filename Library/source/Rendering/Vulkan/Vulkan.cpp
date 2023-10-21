#include "Rendering/Vulkan/Vulkan.h"

DEFINE_LOG_CATEGORY(Vulkan, Info);
DEFINE_LOG_CATEGORY(VulkanMessage, Info);

namespace Rendering {
	const RenderKey RenderKey::Initial{ 1 };
	const RenderKey RenderKey::Invalid{};
}