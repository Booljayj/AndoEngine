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
