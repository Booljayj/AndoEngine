#pragma once
#include <boost/endian/conversion.hpp>
#include "Engine/StandardTypes.h"

//Utility to convert a symbol to a string
#define STRINGIFY(x) #x
//Utility to convert an expanded macro to a string
#define STRINGIFY_MACRO(x) STRINGIFY(x)

/** Expands to a human-readable description of the compiler used when compiling */
#if defined(_MSC_VER)
#define COMPILER_VERSION "Microsoft C/C++ Compiler v" STRINGIFY_MACRO(_MSC_VER)
#elif defined(__clang__)
#define COMPILER_VERSION __VERSION__
#elif defined(__GNUC__)
#define COMPILER_VERSION __VERSION__
#else
#define COMPILER_VERSION "Unknown Compiler"
#endif

namespace Utility {
	/** Load a value from a char array. Char array is assumed to be in little-endian order, and must be large enought to contain the value */
	template<typename T>
	constexpr inline T Load(char const* data, size_t offset = 0) {
		static_assert(std::is_integral_v<T>, "Load must only be used with integral types");
		T value = 0;
		if constexpr (boost::endian::order::native == boost::endian::order::little) {
			for (size_t index = 0; index < sizeof(T); ++index) {
				T const byte = static_cast<T>(*(data + offset + index));
				value |= (byte << (index * 8));
			}
		} else {
			for (size_t index = 0; index < sizeof(T); ++index) {
				T const byte = static_cast<T>(*(data + offset + index));
				value |= (byte << ((sizeof(T) - index - 1) * 8));
			}
		}
		return value;
	}

	/** Write a value to a buffer in reverse order. Return the number of digits written. */
	uint8_t WriteReversedValue(uint64_t value, char* buffer, size_t size);
	/** Write a signed value to a buffer in reverse order. Return the number of digits written. */
	uint8_t WriteReversedValueSigned(int64_t value, char* buffer, size_t size);

	/** Returns the maximum number of characters required to write an integer value with the given type to a string */
	template<typename T>
	constexpr uint8_t MaxCharacters() {
		static_assert(std::numeric_limits<T>::is_integer && std::is_same_v<T, bool>, "MaxCharacters requires an integer type");
		uint8_t count = 0;
		for (size_t value = std::numeric_limits<T>::max(); value > 0; value /= 10) ++count;
		return count + static_cast<uint8_t>(std::is_signed_v<T>);
	}
}

/** A size value in bytes */
struct ByteSize {
	uint64_t size = 0;
	ByteSize(uint64_t inSize) : size(inSize) {}
};

/** Write the value as a size in bytes using an appropriate suffix */
std::ostream& operator<<(std::ostream& stream, ByteSize bytes);
