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
