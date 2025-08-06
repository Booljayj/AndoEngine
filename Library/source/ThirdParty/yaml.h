#pragma once
#include "Engine/Core.h"
#include "yaml-cpp/yaml.h"

//Default serializers for standard types which were missed in the yaml-cpp library

namespace YAML {
	template<>
	struct convert<std::byte> {
		static Node encode(std::byte value) { return convert<uint8_t>::encode(std::to_integer<uint8_t>(value)); }
		static bool decode(Node const& node, std::byte& value) {
			uint8_t byte = 0;
			if (convert<uint8_t>::decode(node, byte)) {
				value = static_cast<std::byte>(byte);
				return true;
			}
			return false;
		}
	};

	template<>
	struct convert<char8_t> {
		static Node encode(char8_t value) { return convert<uint8_t>::encode(static_cast<uint8_t>(value)); }
		static bool decode(Node const& node, char8_t& value) { return convert<uint8_t>::decode(node, reinterpret_cast<uint8_t&>(value)); }
	};
	template<>
	struct convert<char16_t> {
		static Node encode(char16_t value) { return convert<uint16_t>::encode(static_cast<uint16_t>(value)); }
		static bool decode(Node const& node, char16_t& value) { return convert<uint16_t>::decode(node, reinterpret_cast<uint16_t&>(value)); }
	};
	template<>
	struct convert<char32_t> {
		static Node encode(char32_t value) { return convert<uint32_t>::encode(static_cast<uint32_t>(value)); }
		static bool decode(Node const& node, char32_t& value) { return convert<uint32_t>::decode(node, reinterpret_cast<uint32_t&>(value)); }
	};

	template<>
	struct convert<std::u8string> {
		static Node encode(std::u8string const& value);
		static bool decode(Node const& node, std::u8string& value);
	};
	template<>
	struct convert<std::u16string> {
		static Node encode(std::u16string const& value);
		static bool decode(Node const& node, std::u16string& value);
	};
	template<>
	struct convert<std::u32string> {
		static Node encode(std::u32string const& value);
		static bool decode(Node const& node, std::u32string& value);
	};

	template<>
	struct convert<std::u8string_view> {
		static Node encode(std::u8string_view const& value);
	};
	template<>
	struct convert<std::u16string_view> {
		static Node encode(std::u16string_view const& value);
	};
	template<>
	struct convert<std::u32string_view> {
		static Node encode(std::u32string_view const& value);
	};
}
