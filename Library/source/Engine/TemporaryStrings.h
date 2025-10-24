#pragma once
#include "Engine/Core.h"
#include "Engine/String.h"
#include "Engine/StringView.h"
#include "Engine/Temporary.h"

template<typename CharType>
using t_basic_string = std::basic_string<CharType, std::char_traits<CharType>, TTemporaryAllocator<CharType>>;

using t_string = t_basic_string<char>;
using t_wstring = t_basic_string<wchar_t>;
using t_u16string = t_basic_string<char16_t>;
using t_u32string = t_basic_string<char32_t>;
