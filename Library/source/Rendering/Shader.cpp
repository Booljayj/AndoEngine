#include "Rendering/Shader.h"
#include "Engine/Utility.h"
#include "Resources/RegisteredResource.h"

DEFINE_STRUCT_REFLECTION_MEMBERS(Rendering, Shader, "Shader base class", {});
DEFINE_STRUCT_REFLECTION_MEMBERS(Rendering, VertexShader, "Vertex Shader", {});
DEFINE_STRUCT_REFLECTION_MEMBERS(Rendering, FragmentShader, "Fragment Shader", {});

REGISTER_RESOURCE(Rendering, VertexShader);
REGISTER_RESOURCE(Rendering, FragmentShader);

namespace Archive {
	void ShaderSerializer::Write(Output& archive, Rendering::Shader const& shader) {
		archive << shader.bytecode;
	}
	void ShaderSerializer::Read(Input& archive, Rendering::Shader& shader) {
		archive >> shader.bytecode;
	}
}

namespace YAML {
	Node ShaderConverter::encode(Rendering::Shader const& shader) {
		constexpr size_t WordSize = sizeof(decltype(Rendering::Shader::bytecode)::value_type);

		std::vector<unsigned char> characters;
		characters.reserve(shader.bytecode.size() * WordSize);

		for (uint32_t const word : shader.bytecode) {
			//Save the bytes for this word in little-endian format
			std::byte bytes[WordSize];
			Utility::SaveOrdered(word, bytes);

			//Add the bytes for the word to the output
			for (std::byte const byte : bytes) characters.push_back(std::to_integer<char>(byte));
		}

		Node node{ NodeType::Map };
		node["bytecode"] = Node{ EncodeBase64(characters.data(), characters.size()) };
		return node;
	}
	bool ShaderConverter::decode(Node const& node, Rendering::Shader& shader) {
		constexpr size_t WordSize = sizeof(decltype(Rendering::Shader::bytecode)::value_type);

		if (!node.IsMap()) return false;

		std::vector<unsigned char> const characters = DecodeBase64(node["bytecode"].as<std::string>());

		//Ensure the input contains a whole number of words, or the data is malformed.
		if (characters.size() % WordSize) return false;

		//Reserve space for the number of words that will be decoded.
		shader.bytecode.clear();
		shader.bytecode.reserve(characters.size() / WordSize);

		for (size_t index = 0; index < characters.size(); index += WordSize) {
			//Load the bytes for this word in little-endian format
			const auto bytes = std::as_bytes(std::span<unsigned char const, WordSize>{ characters.data() + index, WordSize });
			uint32_t word = 0;
			Utility::LoadOrdered(bytes, word);

			//Add the word to the output
			shader.bytecode.push_back(word);
		}

		return true;
	}
}
