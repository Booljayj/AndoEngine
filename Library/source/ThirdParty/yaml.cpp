#include "ThirdParty/yaml.h"
#include "Engine/StringConversion.h"

namespace YAML {
	Node convert<std::u8string>::encode(std::u8string const& value) {
		return convert<std::string>::encode(ConvertString<std::string>(value));
	}
	bool convert<std::u8string>::decode(Node const& node, std::u8string& value) {
		std::string s;
		if (convert<std::string>::decode(node, s)) {
			ConvertString(s, value);
			return true;
		}
		return false;
	}

	Node convert<std::u16string>::encode(std::u16string const& value) {
		return convert<std::string>::encode(ConvertString<std::string>(value));
	}
	bool convert<std::u16string>::decode(Node const& node, std::u16string& value) {
		std::string s;
		if (convert<std::string>::decode(node, s)) {
			ConvertString(s, value);
			return true;
		}
		return false;
	}

	Node convert<std::u32string>::encode(std::u32string const& value) {
		return convert<std::string>::encode(ConvertString<std::string>(value));
	}
	bool convert<std::u32string>::decode(Node const& node, std::u32string& value) {
		std::string s;
		if (convert<std::string>::decode(node, s)) {
			ConvertString(s, value);
			return true;
		}
		return false;
	}
}
