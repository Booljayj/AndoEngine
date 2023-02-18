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

		std::vector<uint32_t> bytecode;

		Shader(Resources::Identifier id) : Resources::Resource(id) {}
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
