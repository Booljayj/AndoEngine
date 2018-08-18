#pragma once
#include <cstdint>

namespace Geometry {
	/* Flags which specify color channels and various combinations of them */
	enum class FColorChannel : uint8_t {
		None = 0,
		Red = 1 << 0,
		Green = 1 << 1,
		Blue = 1 << 2,

		Yellow = Red | Green,
		Magenta = Red | Blue,
		Cyan = Green | Blue,
		White = Red | Green | Blue,
	};
}
