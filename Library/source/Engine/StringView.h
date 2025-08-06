#pragma once
#include <string_view>
#include "Engine/Concepts.h"
#include "Engine/Core.h"

using namespace std::string_view_literals;

namespace Archive {
	/** Serializer for string_view types */
	template<::Concepts::Character T>
	struct Serializer<std::basic_string_view<T>> {
		static void Write(Output& archive, std::basic_string_view<T> const& string) {
			Serializer<size_t>::Write(archive, string.size());
			for (T const character : string) Serializer<T>::Write(archive, character);
		}
	};
}

//@todo These methods are just snippets to get things working. They assumes the u16 code points are identical to u8 code points, which is only true for some of the more common code points.
//      We'll need a more detailed unicode conversion library to do a proper conversion here, and to do it a non-allocating way.
template<>
struct std::formatter<std::u16string_view> : std::formatter<std::string_view> {
	auto format(std::u16string_view const& s, format_context& ctx) const {
		for (char16_t c : s)
		{
			ctx.out() = static_cast<char>(c);
		}
		return ctx.out();
	}
};
