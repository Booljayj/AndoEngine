#pragma once
#include <ostream>
#include <GL/glew.h>
#include "EntityFramework/Managers/SimpleComponentManager.h"
#include "Rendering/EShader.enum.h"
#include "Rendering/Uniform.h"
#include "Rendering/Types.h"

struct ShaderComponent
{
	//Serialized data
	char const* Source = nullptr;
	GL::EShader::ENUM ShaderType = GL::EShader::Vertex;

	//Runtime data
	ShaderID _ShaderID = 0;
	bool bIsCompiled = false;
};

using ShaderComponentManager = TSimpleComponentManager<ShaderComponent>;

struct ProgramComponent
{
	//Serialized data
	std::vector<ShaderComponent*> LinkedShaders;

	//Runtime data
	ProgramID _ProgramID = 0;
	std::vector<GL::UniformInfo> _Uniforms;
	bool bIsLinked = false;
};

using ProgramComponentManager = TSimpleComponentManager<ProgramComponent>;

namespace GL
{
	bool Compile( ShaderComponent& Shader );
	void DescribeCompilationErrors( std::ostream& Stream, ShaderComponent const& Shader );

	bool Link( ProgramComponent& Program );
	void DescribeLinkingErrors( std::ostream& Stream, ProgramComponent const& Program );

	void Use( ProgramComponent const& Program );
}
