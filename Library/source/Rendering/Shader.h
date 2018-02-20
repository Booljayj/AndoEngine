#pragma once
#include <ostream>
#include <GL/glew.h>
#include "EntityFramework/Managers/SimpleComponentManager.h"
#include "Rendering/EShader.enum.h"
#include "Rendering/Uniform.h"
#include "Rendering/Types.h"

namespace C
{
	struct ShaderComponent
	{
		//Serialized data
		const char* Source = nullptr;
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
}

namespace GL
{
	bool Compile( C::ShaderComponent& Shader );
	void DescribeCompilationErrors( std::ostream& Stream, const C::ShaderComponent& Shader );

	bool Link( C::ProgramComponent& Program );
	void DescribeLinkingErrors( std::ostream& Stream, const C::ProgramComponent& Program );

	void Use( const C::ProgramComponent& Program );
}
