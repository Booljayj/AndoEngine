#pragma once
#include "Engine/String.h"

/** Convert a string in one encoding to a string in a different encoding, possibly expanding or contracting multi-byte encodings */
template<typename ToType, typename FromType>
void ConvertString(FromType const&, ToType&);

/** Convert a string in one encoding to a string in a different encoding, possibly expanding or contracting multi-byte encodings */
template<typename ToType, typename FromType>
inline ToType ConvertString(FromType const& f) { ToType t; ConvertString<ToType, FromType>(f, t); return t; }

template<>
void ConvertString<std::string, std::u8string>(std::u8string const& f, std::string& t);

template<>
void ConvertString<std::string, std::u16string>(std::u16string const& f, std::string& t);

template<>
void ConvertString<std::string, std::u32string>(std::u32string const& f, std::string& t);

template<>
void ConvertString<std::u8string, std::string>(std::string const& f, std::u8string& t);

template<>
void ConvertString<std::u16string, std::string>(std::string const& f, std::u16string& t);

template<>
void ConvertString<std::u32string, std::string>(std::string const& f, std::u32string& t);
