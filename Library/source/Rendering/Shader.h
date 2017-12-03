#pragma once
#include <ostream>
#include <GL/glew.h>
#include "EntityFramework/ComponentManager.h"
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
		ProgramID _ProgramID = 0;
		std::vector<GL::UniformInfo> _Uniforms;
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
	bool Compile( C::ShaderComponent& Shader );
	void DescribeCompilationErrors( std::ostream& Stream, const C::ShaderComponent& Shader );

	bool Link( C::ProgramComponent& Program );
	void DescribeLinkingErrors( std::ostream& Stream, const C::ProgramComponent& Program );

	void Use( const C::ProgramComponent& Program );
}