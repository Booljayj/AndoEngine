#include <iostream>
#include <vector>
#include "Rendering/EGLBool.enum.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexArrayObject.h"

namespace GL
{
	bool Compile( ShaderComponent& Shader )
	{
		if( Shader.bIsCompiled ) {
			//Shader is already compiled.
			return true;
		}

		//Create a new OpenGL Shader object if we need one.
		if( Shader._ShaderID == 0 ) {
			Shader._ShaderID = glCreateShader( EShader::ToGL( Shader.ShaderType ) );
		}
		glShaderSource( Shader._ShaderID, 1, &Shader.Source, 0 );

		GLint CompileStatus;
		glCompileShader( Shader._ShaderID );
		glGetShaderiv( Shader._ShaderID, GL_COMPILE_STATUS, &CompileStatus );

		if( !(Shader.bIsCompiled = EGLBool::FromGL( CompileStatus ) == EGLBool::True ) ) {
			DescribeCompilationErrors( std::cerr, Shader );
		}

		return Shader.bIsCompiled;
	}

	void DescribeCompilationErrors( std::ostream& Stream, ShaderComponent const& Shader )
	{
		GLint MsgLength = 0;
		glGetShaderiv( Shader._ShaderID, GL_INFO_LOG_LENGTH, &MsgLength );
		char* const MsgBuffer = new char[MsgLength];
		glGetShaderInfoLog( Shader._ShaderID, MsgLength, &MsgLength, MsgBuffer );

		Stream << "ERROR [OpenGL] Compiling Shader " << Shader._ShaderID << ": " << MsgBuffer << std::endl;
		delete[] MsgBuffer;
	}

	bool Link( ProgramComponent& Program )
	{
		GLint LinkStatus;

		if( Program.bIsLinked ) {
			//Program is already linked
			return true;
		}

		//Create a program object in OpenGL if we need a new one
		if( Program._ProgramID == 0 ) {
			Program._ProgramID = glCreateProgram();
		}

		//Attempt to load precompiled program data
//		if( Program._Binary )
//		{
//			glProgramBinary( Program._ProgramID, Program._BinaryVersion, Program._Binary.size(), Program._Binary.data() );
//			glGetProgramiv( Program._ProgramID, GL_LINK_STATUS, &LinkStatus );
//			bLinkingWasSuccessful = EGLBool::FromGlobal( LinkStatus ) == EGLBool::True;
//
//			if( bLinkingWasSuccessful )
//			{
//				//The program was loaded from an existing binary. Huzzah.
//				return true;
//			}
//		}

		//Compile and attach any shaders this program depends on.
		for( ShaderComponent* Shader : Program.LinkedShaders ) {
			if( Shader && GL::Compile( *Shader ) ) {
				glAttachShader( Program._ProgramID, Shader->_ShaderID );
			}
		}

		//Add attribute location information to the program
		BindAttributeNames( Program._ProgramID );

		//Do the actual linking
		glLinkProgram( Program._ProgramID );

		//Determing if the linking was successful (saving result to component)
		glGetProgramiv( Program._ProgramID, GL_LINK_STATUS, &LinkStatus );

		if( ( Program.bIsLinked = EGLBool::FromGL( LinkStatus ) == EGLBool::True ) ) {
			GetUniforms( Program._ProgramID, Program._Uniforms );
		} else {
			DescribeLinkingErrors( std::cerr, Program );
		}

		//Detach shaders before returning success result
		for( ShaderComponent* Shader : Program.LinkedShaders ) {
			if( Shader && Shader->bIsCompiled ) {
				glDetachShader( Program._ProgramID, Shader->_ShaderID );
				//In the future, this can be delayed until all programs are linked
				glDeleteShader( Shader->_ShaderID );
			}
		}

		return Program.bIsLinked;
	}

	void DescribeLinkingErrors( std::ostream& Stream, ProgramComponent const& Program )
	{
		GLint MsgLength = 0;
		glGetProgramiv( Program._ProgramID, GL_INFO_LOG_LENGTH, &MsgLength );
		char* const MsgBuffer = new char[MsgLength];
		glGetProgramInfoLog( Program._ProgramID, MsgLength, &MsgLength, MsgBuffer );

		Stream << "ERROR [OpenGL] Linking Program " << Program._ProgramID << ": " << MsgBuffer << std::endl;
		delete[] MsgBuffer;
	}

	void Use( ProgramComponent const& Program )
	{
		glUseProgram( Program._ProgramID );
	}
}
