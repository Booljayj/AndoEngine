#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>
#include "Engine/Logging.h"
#include "Engine/StandardTypes.h"
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

template<>
struct std::formatter<Rendering::VulkanVersion> : std::formatter<std::string_view> {
	auto format(const Rendering::VulkanVersion& version, format_context& ctx) const {
		return std::format_to(ctx.out(), "{0}.{1}.{2}"sv, version.major, version.minor, version.patch);
	}
};
