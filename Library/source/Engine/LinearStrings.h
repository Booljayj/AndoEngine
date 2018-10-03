#pragma once
#include <string>
#include <string_view>
#include <cstdio>
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"

template< class CharT >
using l_basic_string = std::basic_string<CharT, std::char_traits<CharT>, TLinearAllocator<CharT>>;

using l_string = l_basic_string<char>;
using l_wstring = l_basic_string<wchar_t>;
using l_u16string = l_basic_string<char16_t>;
using l_u32string = l_basic_string<char32_t>;

std::string_view l_printf( HeapBuffer& Allocator, char const* Format, ... );
