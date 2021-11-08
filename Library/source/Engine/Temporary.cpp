#include "Engine/Temporary.h"

thread_local Buffer* threadTemporaryBuffer = nullptr;

std::string_view t_printf(char const* format, ...) {
	std::va_list args;
	va_start(args, format);

	Buffer& buffer = *threadTemporaryBuffer;
	size_t const available = buffer.GetAvailable();
	char* const begin = buffer.GetCursor();
	size_t const size = vsnprintf(begin, available, format, args);
	//Set the cursor to the byte after the null terminator so subsequent prints don't interfere with the results of this one.
	buffer.SetCursor(begin + size + 1);

	va_end(args);
	return std::string_view{ begin, size };
}
