#include <algorithm>
#include <cstdarg>
#include <cstring>
#include "Engine/LinearStrings.h"

std::string_view l_printf( HeapBuffer& Buffer, char const* Format, ... )
{
	std::va_list args;
	va_start( args, Format );

	size_t const MaxAvailableAllocatorCapacity = Buffer.GetAvailable();
	char* const FormattedStringPtr = Buffer.GetCursor();
	size_t const FormattedStringLength = vsnprintf( FormattedStringPtr, MaxAvailableAllocatorCapacity, Format, args );
	Buffer.SetCursor( FormattedStringPtr + FormattedStringLength );

	va_end( args );
	return std::string_view{ FormattedStringPtr, FormattedStringLength };
}
