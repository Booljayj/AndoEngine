#pragma once
#include <string>
#include "Engine/Core.h"
#include "Engine/Reflection/TypeInfo.h"
#include "Engine/StringView.h"

namespace Archive {
	template<typename T, typename AllocatorType>
	struct Serializer<std::basic_string<T, std::char_traits<T>, AllocatorType>> {
		static void Write(Output& archive, std::basic_string<T, std::char_traits<T>, AllocatorType> const& string) {
			Serializer<size_t>::Write(archive, string.size());
			for (T const character : string) Serializer<T>::Write(archive, character);
		}
		static void Read(Input& archive, std::basic_string<T, std::char_traits<T>, AllocatorType>& string) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);

			string.resize(num);
			for (T& character : string) Serializer<T>::Read(archive, character);
		}
	};
}

REFLECT(std::string, String);
REFLECT(std::u8string, String);
REFLECT(std::u16string, String);
REFLECT(std::u32string, String);

//@todo These methods are just snippets to get things working. They assumes the u16 code points are identical to u8 code points, which is only true for some of the more common code points.
//      We'll need a more detailed unicode conversion library to do a proper conversion here, and to do it a non-allocating way.
template<>
struct std::formatter<char16_t> : std::formatter<char> {
	auto format(char16_t c, format_context& ctx) const {
		return formatter<char>::format(static_cast<char>(c), ctx);
	}
};
template<>
struct std::formatter<std::u16string> : std::formatter<std::string> {
	auto format(std::u16string const& s, format_context& ctx) const {
		for (char16_t c : s)
		{
			ctx.out() = static_cast<char>(c);
		}
		return ctx.out();
	}
};
