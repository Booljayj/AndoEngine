#pragma once
#include "Engine/String.h"

void ConvertString(std::u8string_view f, std::string& t);
void ConvertString(std::u16string_view f, std::string& t);
void ConvertString(std::u32string_view f, std::string& t);
void ConvertString(std::string_view f, std::u8string& t);
void ConvertString(std::string_view f, std::u16string& t);
void ConvertString(std::string_view f, std::u32string& t);

/** Convert a string in one encoding to a string in a different encoding, possibly expanding or contracting multi-byte encodings */
template<typename ToCharacterType, typename FromCharacterType>
inline std::basic_string<ToCharacterType> ConvertString(std::basic_string_view<FromCharacterType> f) { std::basic_string<ToCharacterType> t; ConvertString(f, t); return t; }
/** Convert a string in one encoding to a string in a different encoding, possibly expanding or contracting multi-byte encodings */
template<typename ToCharacterType, typename FromCharacterType, typename AllocatorType>
inline std::basic_string<ToCharacterType> ConvertString(std::basic_string<FromCharacterType, std::char_traits<FromCharacterType>, AllocatorType> const& f) { return ConvertString<ToCharacterType>(std::basic_string_view<FromCharacterType>{ f }); }
