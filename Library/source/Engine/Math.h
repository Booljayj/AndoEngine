#pragma once
#include "Engine/STL.h"

//Advanced math functions not available in standard libraries
namespace Math {
	//Circular shift left
	template<typename T>
	constexpr inline T RotateLeft(const T x, uint32_t n) noexcept {
		static_assert(std::is_integral<T>::value, "Cannot rotate non-integral type");
		static_assert(!std::is_signed<T>::value, "Cannot rotate signed type");

		constexpr uint32_t NumBits = std::numeric_limits<T>::digits;
		constexpr uint32_t Mask = NumBits - 1;
		static_assert((NumBits & Mask) == 0, "Integral type has a non power-of-two size, cannot rotate" );

		n &= Mask;
		return (x << n) | (x >> (-n & Mask));
	}

	//Circular shift right
	template<typename T>
	constexpr inline T RotateRight(const T x, uint32_t n) noexcept  {
		static_assert(std::is_integral<T>::value, "Cannot rotate non-integral type");
		static_assert(!std::is_signed<T>::value, "Cannot rotate signed type");

		constexpr uint32_t NumBits = std::numeric_limits<T>::digits;
		constexpr uint32_t Mask = NumBits - 1;
		static_assert((NumBits & Mask) == 0, "Integral type has a non power-of-two size, cannot rotate" );

		n &= Mask;
		return (x >> n) | (x << (-n & Mask));
	}
}
