#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>
#include "Engine/Logging.h"
#include "Engine/StandardTypes.h"
#include "ThirdParty/vk_mem_alloc.h"

DECLARE_LOG_CATEGORY(Vulkan);
DECLARE_LOG_CATEGORY(VulkanMessage);

namespace Rendering {
	/** Type used as a key value to mark resources that depend on an ongoing process */
	struct RenderKey {
		/** The initial valid value to use for a key */
		static const RenderKey Initial;
		/** An invalid key, used to mark resources that haven't been used at all yet */
		static const RenderKey Invalid;
	
		RenderKey() = default;
		RenderKey(const RenderKey&) = default;
		inline bool operator==(RenderKey const& other) const { return value == other.value; }

		/** Modity this key to the next valid key */
		inline void Increment() { value += 2; }

	private:
		friend std::hash<RenderKey>;
		uint32_t value = 0;
		RenderKey(uint32_t inValue) : value(inValue) {}
	};

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

template<>
struct std::hash<Rendering::RenderKey> {
	size_t operator()(Rendering::RenderKey const& key) const {
		return std::hash<uint32_t>{}(key.value);
	}
};
