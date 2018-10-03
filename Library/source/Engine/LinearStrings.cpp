#include <algorithm>
#include <cstdarg>
#include <cstring>
#include "Engine/LinearStrings.h"

std::string_view l_printf( LinearAllocatorData& Allocator, char const* Format, ... )
{
	std::va_list args;
	va_start( args, Format );

	size_t const MaxAvailableAllocatorCapacity = Allocator.GetCapacity() - Allocator.GetUsed();
	char* const FormattedStringPtr = reinterpret_cast<char*>( Allocator.GetData( Allocator.GetUsed() ) );
	size_t const FormattedStringLength = vsnprintf( FormattedStringPtr, MaxAvailableAllocatorCapacity, Format, args );
	Allocator.SetUsed( Allocator.GetUsed() + FormattedStringLength );

	va_end( args );
	return std::string_view{ FormattedStringPtr, FormattedStringLength };
}
