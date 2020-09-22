#pragma once
#include "Engine/STL.h"

namespace Rendering {
	/** Wrapper for a Vulkan-style version number to make it easier to print it out to a stream */
	struct VulkanVersion {
		const uint32_t patch : 12;
		const uint32_t minor : 10;
		const uint32_t major : 10;

		VulkanVersion(const uint32_t& value)
		: patch(0)
		, minor(0)
		, major(0)
		{
			static_assert(sizeof(VulkanVersion) == sizeof(uint32_t), "Bits of VulkanVersion must be directly copyable from a uint32_t");
			memcpy(this, &value, sizeof(uint32_t));
		}
	};
}

inline std::ostream& operator<<(std::ostream& stream, Rendering::VulkanVersion const& version) {
	stream << version.major << "." << version.minor << "." << version.patch;
	return stream;
}
