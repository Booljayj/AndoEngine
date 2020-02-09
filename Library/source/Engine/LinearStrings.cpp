#include <algorithm>
#include <cstdarg>
#include <cstring>
#include "Engine/LinearStrings.h"

std::string_view l_printf(HeapBuffer& buffer, char const* format, ...) {
	std::va_list args;
	va_start(args, format);

	size_t const maxAvailableAllocatorCapacity = buffer.GetAvailable();
	char* const formattedStringPtr = buffer.GetCursor();
	size_t const formattedStringLength = vsnprintf(formattedStringPtr, maxAvailableAllocatorCapacity, format, args);
	buffer.SetCursor(formattedStringPtr + formattedStringLength);

	va_end(args);
	return std::string_view{formattedStringPtr, formattedStringLength};
}
