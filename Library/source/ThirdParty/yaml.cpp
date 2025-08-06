#include "ThirdParty/yaml.h"
#include "Engine/StringConversion.h"

namespace YAML {
	Node convert<std::u8string>::encode(std::u8string const& value) {
		return convert<std::string>::encode(ConvertString<char>(value));
	}
	bool convert<std::u8string>::decode(Node const& node, std::u8string& value) {
		std::string_view s;
		if (convert<std::string_view>::decode(node, s)) {
			ConvertString(s, value);
			return true;
		}
		return false;
	}

	Node convert<std::u16string>::encode(std::u16string const& value) {
		return convert<std::string>::encode(ConvertString<char>(value));
	}
	bool convert<std::u16string>::decode(Node const& node, std::u16string& value) {
		std::string_view s;
		if (convert<std::string_view>::decode(node, s)) {
			ConvertString(s, value);
			return true;
		}
		return false;
	}

	Node convert<std::u32string>::encode(std::u32string const& value) {
		return convert<std::string>::encode(ConvertString<char>(value));
	}
	bool convert<std::u32string>::decode(Node const& node, std::u32string& value) {
		std::string_view s;
		if (convert<std::string_view>::decode(node, s)) {
			ConvertString(s, value);
			return true;
		}
		return false;
	}

	Node convert<std::u8string_view>::encode(std::u8string_view const& value) {
		return convert<std::string>::encode(ConvertString<char>(value));
	}

	Node convert<std::u16string_view>::encode(std::u16string_view const& value) {
		return convert<std::string>::encode(ConvertString<char>(value));
	}

	Node convert<std::u32string_view>::encode(std::u32string_view const& value) {
		return convert<std::string>::encode(ConvertString<char>(value));
	}
}
