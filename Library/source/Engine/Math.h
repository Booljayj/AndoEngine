#pragma once
#include <cmath>
#include "Engine/Core.h"

//Advanced math functions not available in standard libraries
namespace Math {
	/** Calculates floor(log2(value)) */
	constexpr size_t FloorLog2(size_t value) {
		return value == 1 ? 0 : 1 + FloorLog2(value >> 1);
	}

	/** Calculates the minimum number of bits required to store a maximum value. Also equal to ceil(log2(value)). */
	constexpr size_t GetMinimumNumBits(size_t max) {
		return max == 1 ? 0 : FloorLog2(max - 1) + 1;
	}
}
