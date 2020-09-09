#pragma once
#include <ostream>
#include <GL/glew.h>
#include "Rendering/EShader.enum.h"
#include "Rendering/Uniform.h"
#include "Rendering/Types.h"

struct ShaderComponent {
	//Serialized data
	char const* source = nullptr;
	GL::EShader::ENUM shaderType = GL::EShader::Vertex;

	//Runtime data
	ShaderID shaderID = 0;
	bool isCompiled = false;
};

struct ProgramComponent {
	//Serialized data
	std::vector<ShaderComponent*> linkedShaders;

	//Runtime data
	ProgramID programID = 0;
	std::vector<GL::UniformInfo> uniforms;
	bool isLinked = false;
};

namespace GL {
	bool Compile(ShaderComponent& shader);
	void DescribeCompilationErrors(std::ostream& stream, ShaderComponent const& shader);

	bool Link(ProgramComponent& program);
	void DescribeLinkingErrors(std::ostream& stream, ProgramComponent const& program);

	void Use(ProgramComponent const& program);
}
