#pragma once

#include <ostream>
#include "GL/glew.h"

#include "EntityFramework/ComponentManager.h"
#include "Rendering/Shader.enum.h"
#include "Rendering/GLUniform.h"

namespace GL
{
	using ProgramID = GLuint;
	using ShaderID = GLuint;
}

namespace C
{
	using namespace std;

	struct ShaderComponent
	{
		//Serialized data
		const char* Source = nullptr;
		GL::EShader::ENUM ShaderType = GL::EShader::Vertex;

		//Runtime data
		GL::ShaderID _ShaderID = 0;
		bool bIsCompiled = false;
	};

	class ShaderComponentManager : public TComponentManager<ShaderComponent>
	{
		void OnRetained( ShaderComponent* Comp ) override final {}
		void OnReleased( ShaderComponent* Comp ) override final {}
	};

	struct ProgramComponent
	{
		//Serialized data
		std::vector<ShaderComponent*> LinkedShaders;

		//Runtime data
		GL::ProgramID _ProgramID = 0;
		vector<GL::UniformInfo> _Uniforms;
		bool bIsLinked = false;
	};

	class ProgramComponentManager : public TComponentManager<ProgramComponent>
	{
		void OnRetained( ProgramComponent* Comp ) override final {}
		void OnReleased( ProgramComponent* Comp ) override final {}
	};
}

namespace GL
{
	using namespace std;

	bool Compile( C::ShaderComponent& Shader );
	void DescribeCompilationErrors( ostream& Stream, const C::ShaderComponent& Shader );

	bool Link( C::ProgramComponent& Program );
	void DescribeLinkingErrors( ostream& Stream, const C::ProgramComponent& Program );

	void Use( const C::ProgramComponent& Program );
}
