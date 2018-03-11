#pragma once
#include <string>
#include <cstdio>
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"

template< class CharT >
using l_basic_string = std::basic_string<CharT, std::char_traits<CharT>, TLinearAllocator<CharT>>;

using l_string = l_basic_string<char>;
using l_wstring = l_basic_string<wchar_t>;
using l_u16string = l_basic_string<char16_t>;
using l_u32string = l_basic_string<char32_t>;

char const* l_printf( LinearAllocatorData& Alloc, char const* Format, ... );
l_string l_sprintf( LinearAllocatorData& Alloc, char const* Format, ... );

//@todo Create some pretty comprehensive tests. It's very possible that this will go completely insane and start writing to invalid memory,
// unless I got really lucky and managed to implement everything perfectly first try.

/** A class used to construct a string from an arbitrary number of write operations */
class l_string_builder
{
	static constexpr uint32_t BLOCK_SIZE = 32;

	LinearAllocatorData& Alloc;
	char* Position = nullptr;
	l_list<char*> Blocks;

public:
	l_string_builder( LinearAllocatorData& InAlloc );

	l_string_builder& operator<<( char const* Message );

	char const* Finish();

protected:
	char* AllocateNewBlock();
};
