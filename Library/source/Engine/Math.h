#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>

//Advanced math functions not available in standard libraries
namespace Math {
	//Circular shift left
	template<typename T>
	constexpr inline T RotateLeft( const T X, uint32_t N ) noexcept {
		static_assert(std::is_integral<T>::value, "Cannot rotate non-integral type");
		static_assert(!std::is_signed<T>::value, "Cannot rotate signed type");

		constexpr uint32_t NumBits = std::numeric_limits<T>::digits;
		constexpr uint32_t Mask = NumBits - 1;
		static_assert((NumBits & Mask) == 0, "Integral type has a non power-of-two size, cannot rotate" );

		N &= Mask;
		return (X << N) | (X >> (-N & Mask));
	}

	//Circular shift right
	template<typename T>
	constexpr inline T RotateRight( const T X, uint32_t N ) noexcept  {
		static_assert(std::is_integral<T>::value, "Cannot rotate non-integral type");
		static_assert(!std::is_signed<T>::value, "Cannot rotate signed type");

		constexpr uint32_t NumBits = std::numeric_limits<T>::digits;
		constexpr uint32_t Mask = NumBits - 1;
		static_assert((NumBits & Mask) == 0, "Integral type has a non power-of-two size, cannot rotate" );

		N &= Mask;
		return (X >> N) | (X << (-N & Mask));
	}
}
