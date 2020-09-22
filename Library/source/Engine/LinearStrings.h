#pragma once
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/STL.h"

template<class CharType>
using l_basic_string = std::basic_string<CharType, std::char_traits<CharType>, TLinearAllocator<CharType>>;

using l_string = l_basic_string<char>;
using l_wstring = l_basic_string<wchar_t>;
using l_u16string = l_basic_string<char16_t>;
using l_u32string = l_basic_string<char32_t>;

std::string_view l_printf(HeapBuffer& buffer, char const* format, ...);
