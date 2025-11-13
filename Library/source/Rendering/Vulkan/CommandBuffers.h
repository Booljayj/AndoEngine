#pragma once
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	enum class ECommandBufferLevel : std::underlying_type_t<VkCommandBufferLevel> {
		Primary = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
	};
}
