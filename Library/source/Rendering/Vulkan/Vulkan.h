#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>
#include "Engine/Logging/LogCategory.h"
#include "Engine/STL.h"
#include "ThirdParty/vk_mem_alloc.h"

DECLARE_LOG_CATEGORY(Vulkan);
DECLARE_LOG_CATEGORY(VulkanMessage);

namespace Rendering {
	/** Wrapper for a Vulkan-style version number to make it easier to print it out to a stream */
	struct VulkanVersion {
		const uint32_t patch : 12;
		const uint32_t minor : 10;
		const uint32_t major : 10;

		VulkanVersion() : patch(0), minor(0), major(0) {}
		VulkanVersion(uint32_t value) : VulkanVersion() { *this = value; }
		VulkanVersion& operator=(uint32_t value) {
			static_assert(sizeof(VulkanVersion) == sizeof(uint32_t), "Bits of VulkanVersion must be directly copyable from a uint32_t");
			memcpy(this, &value, sizeof(uint32_t));
			return *this;
		}
		operator uint32_t() const { return *reinterpret_cast<uint32_t const*>(this); }
	};
}

inline std::ostream& operator<<(std::ostream& stream, Rendering::VulkanVersion const& version) {
	stream << version.major << '.' << version.minor << '.' << version.patch;
	return stream;
}
