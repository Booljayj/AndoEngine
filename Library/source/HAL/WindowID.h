#pragma once
#include "Engine/Core.h"

namespace HAL {
	struct WindowID {
		constexpr WindowID() = default;
		constexpr WindowID(WindowID const&) = default;

		explicit constexpr WindowID(uint32_t id) : id(id) {}

		inline operator bool() const { return id != 0; }
		inline explicit operator uint32_t() const { return id; }
		inline bool operator==(WindowID other) const { return other.id == id; }
		inline bool operator!=(WindowID other) const { return other.id != id; }

	private:
		uint32_t id = 0;
	};
}

template<>
struct std::hash<HAL::WindowID> {
	size_t operator()(HAL::WindowID value) const {
		return static_cast<uint32_t>(value);
	}
};

template<>
struct std::formatter<HAL::WindowID> : std::formatter<uint32_t> {
	auto format(HAL::WindowID value, format_context& ctx) const {
		return formatter<uint32_t>::format(static_cast<uint32_t>(value), ctx);
	}
};
