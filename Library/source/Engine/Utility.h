#pragma once
#include "Engine/STL.h"

//Utility to convert a symbol to a string
#define STRINGIFY(x) #x
//Utility to convert an expanded macro to a string
#define STRINGIFY_MACRO(x) STRINGIFY(x)

namespace Utility {
	/** Write a value to a buffer using an exact number of digits in reverse order. */
	void WriteReversedValue(uint16_t value, const uint8_t numDigits, char* buffer);
	/** Write a value to a buffer using a maximum number of digits in reverse order. Returns the number of digits actually written. */
	uint8_t WriteMinimalReversedValue(uint16_t value, const uint8_t maxNumDigits, char* buffer);

	/** Write the value as a size in bytes using an appropriate suffix */
	void WriteByteSizeValue(std::ostream& stream, uint64_t size);
}
