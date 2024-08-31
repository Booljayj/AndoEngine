#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/Reflection.h"
#include "Resources/Resource.h"

namespace Rendering {
	enum class EShaderType : uint8_t {
		Vertex,
		Fragment,
		Compute,
		Geometry,
		TesselationControl,
		TesselationEvaluation,
	};

	struct Shader : public Resources::Resource {
		REFLECT_STRUCT(Shader, Resources::Resource);
		using Resources::Resource::Resource;

		std::vector<uint32_t> bytecode;

		virtual EShaderType GetShaderType() const = 0;
	};

	struct VertexShader : public Shader {
		REFLECT_STRUCT(VertexShader, Shader);
		using Shader::Shader;

		virtual EShaderType GetShaderType() const override { return EShaderType::Vertex; }
	};
	struct FragmentShader : public Shader {
		REFLECT_STRUCT(FragmentShader, Shader);
		using Shader::Shader;

		virtual EShaderType GetShaderType() const override { return EShaderType::Fragment; }
	};
}

REFLECT(Rendering::Shader, Struct);
REFLECT(Rendering::VertexShader, Struct);
REFLECT(Rendering::FragmentShader, Struct);

//Custom YAML serialization for shader bytecode
namespace YAML {
	template<>
	struct convert<Rendering::Shader> {
		static Node encode(Rendering::Shader const& shader);
		static bool decode(Node const& node, Rendering::Shader& shader);
	};

	template<>
	struct convert<Rendering::VertexShader> : public convert<Rendering::Shader> {};
	template<>
	struct convert<Rendering::FragmentShader> : public convert<Rendering::Shader> {};
}
