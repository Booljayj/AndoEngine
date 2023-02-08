#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/Reflection.h"
#include "Resources/Database.h"
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

		virtual EShaderType GetShaderType() const = 0;
	};

	struct VertexShader : public Shader {
		REFLECT_STRUCT(VertexShader, Shader);

		virtual EShaderType GetShaderType() const override { return EShaderType::Vertex; }
	};
	struct FragmentShader : public Shader {
		REFLECT_STRUCT(FragmentShader, Shader);

		virtual EShaderType GetShaderType() const override { return EShaderType::Fragment; }
	};

	struct ShaderDatabase : public Resources::TSparseDatabase<Shader, ShaderDatabase> {
		using Resources::TSparseDatabase<Shader, ShaderDatabase>::TSparseDatabase;
		void PostCreate(Shader& Resource) {}
	};
}

REFLECT(Rendering::Shader, Struct);
REFLECT(Rendering::VertexShader, Struct);
REFLECT(Rendering::FragmentShader, Struct);
