#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
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
		DECLARE_STRUCT_REFLECTION_MEMBERS(Shader, Resources::Resource);
		using Resources::Resource::Resource;

		std::vector<uint32_t> bytecode;

		virtual EShaderType GetShaderType() const = 0;
	};

	struct VertexShader : public Shader {
		DECLARE_STRUCT_REFLECTION_MEMBERS(VertexShader, Shader);
		using Shader::Shader;

		virtual EShaderType GetShaderType() const override { return EShaderType::Vertex; }
	};
	struct FragmentShader : public Shader {
		DECLARE_STRUCT_REFLECTION_MEMBERS(FragmentShader, Shader);
		using Shader::Shader;

		virtual EShaderType GetShaderType() const override { return EShaderType::Fragment; }
	};
}

REFLECT(Rendering::Shader, Struct);
REFLECT(Rendering::VertexShader, Struct);
REFLECT(Rendering::FragmentShader, Struct);

//Custom archive serialization for shader bytecode
namespace Archive {
	struct ShaderSerializer {
		static void Write(Output& archive, Rendering::Shader const& shader);
		static void Read(Input& archive, Rendering::Shader& shader);
	};

	template<std::derived_from<Rendering::Shader> ShaderType>
	struct Serializer<ShaderType> : public ShaderSerializer {};
}

//Custom YAML serialization for shader bytecode
namespace YAML {
	struct ShaderConverter {
		static Node encode(Rendering::Shader const& shader);
		static bool decode(Node const& node, Rendering::Shader& shader);
	};

	template<std::derived_from<Rendering::Shader> ShaderType>
	struct convert<ShaderType> : public ShaderConverter {};
}
