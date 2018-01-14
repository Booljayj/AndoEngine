#pragma once
#include <string>
#include <cstdarg>
#include <cstdio>
#include "Engine/LinearAllocator.h"

template< class CharT >
using l_basic_string = std::basic_string<CharT, std::char_traits<CharT>, TLinearAllocator<CharT>>;

using l_string = l_basic_string<char>;
using l_wstring = l_basic_string<wchar_t>;
using l_u16string = l_basic_string<char16_t>;
using l_u32string = l_basic_string<char32_t>;

const char* l_printf( LinearAllocatorData& Allocator, const char* Format, ... )
{
	char* const DataPtr = reinterpret_cast<char*>( Allocator.GetData( Allocator.GetUsed() ) );
	const size_t MaxLength = Allocator.GetCapacity() - Allocator.GetUsed();
	va_list args;
	va_start( args, Format );

	const size_t ActualLength = vsnprintf( DataPtr, MaxLength, Format, args );
	Allocator.SetUsed( Allocator.GetUsed() + ActualLength );

	va_end( args );
	return DataPtr;
}
