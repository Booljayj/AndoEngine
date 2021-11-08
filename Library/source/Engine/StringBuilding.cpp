#include "Engine/StringBuilding.h"
#include "Engine/Utility.h"

std::string_view l_printf(Buffer& buffer, char const* format, ...) {
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

StringBuilder& StringBuilder::operator<<(std::string_view const start) {
	size_t const available = buffer.GetAvailable();
	char* const begin = buffer.GetCursor();
	size_t const size = std::min(available, start.size());

	std::memcpy(begin, start.data(), size);
	*(begin + size) = '\0';
	//Move the cursor over the null terminator, so the next written start will overwrite it.
	buffer.SetCursor(begin + size);
	return *this;
}

StringBuilder& StringBuilder::Write(char const* format, ...) {
	std::va_list args;
	va_start(args, format);

	size_t const available = buffer.GetAvailable();
	char* const begin = buffer.GetCursor();
	size_t const size = vsnprintf(begin, available, format, args);
	//Move the cursor over the null terminator, so the next written start will overwrite it.
	buffer.SetCursor(begin + size);

	va_end(args);
	return *this;
}
