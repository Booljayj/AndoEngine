#include "Engine/LinearStrings.h"

std::string_view l_printf(HeapBuffer& buffer, char const* format, ...) {
	std::va_list args;
	va_start(args, format);

	size_t const available = buffer.GetAvailable();
	char* const begin = buffer.GetCursor();
	size_t const size = vsnprintf(begin, available, format, args);
	//Set the cursor to the byte after the null terminator so subsequent prints don't interfere with the results of this one.
	buffer.SetCursor(begin + size + 1);

	va_end(args);
	return std::string_view{ begin, size };
}
