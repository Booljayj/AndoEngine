#pragma once
#include "Engine/STL.h"

//Utility to convert a symbol to a string
#define STRINGIFY(x) #x
//Utility to convert an expanded macro to a string
#define STRINGIFY_MACRO(x) STRINGIFY(x)

namespace Utility {
	/** Write a value to a buffer in reverse order. Return the number of digits written. */
	uint8_t WriteReversedValue(uint64_t value, char* buffer, size_t size);
	/** Write a signed value to a buffer in reverse order. Return the number of digits written. */
	uint8_t WriteReversedValue(int64_t value, char* buffer, size_t size);

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
