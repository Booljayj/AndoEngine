//
//  GLShader.cpp
//  AndoEngine
//
//  Created by Justin Bool on 9/5/17.
//
//

#include "GLShader.h"
#include <iostream>
#include <vector>

#include "GLBasicEnums.h"
#include "GLVertexArrayObject.h"


namespace GL
{
	DefineEnumerationConverter( EShader );

	bool Compile( C::ShaderComponent& Shader )
	{
		if( Shader.bIsCompiled )
		{
			//Shader is already compiled.
			return true;
		}

		//Create a new OpenGL Shader object if we need one.
		if( Shader._ShaderID == 0 )
		{
			Shader._ShaderID = glCreateShader( EShader::ToGlobal( Shader.ShaderType ) );
		}
		glShaderSource( Shader._ShaderID, 1, &Shader.Source, 0 );

		GLint CompileStatus;
		glCompileShader( Shader._ShaderID );
		glGetShaderiv( Shader._ShaderID, GL_COMPILE_STATUS, &CompileStatus );

		if( !(Shader.bIsCompiled = EGLBool::FromGlobal( CompileStatus ) == EGLBool::True ) )
		{
			DescribeCompilationErrors( std::cerr, Shader );
		}

		return Shader.bIsCompiled;
	}

	void DescribeCompilationErrors( ostream& Stream, const C::ShaderComponent& Shader )
	{
		GLint MsgLength = 0;
		char* MsgBuffer;
		glGetShaderiv( Shader._ShaderID, GL_INFO_LOG_LENGTH, &MsgLength );
		MsgBuffer = new char[MsgLength];
		glGetShaderInfoLog( Shader._ShaderID, MsgLength, &MsgLength, MsgBuffer );

		Stream << "ERROR [OpenGL] Compiling Shader " << Shader._ShaderID << ": " << MsgBuffer << endl;
		delete[] MsgBuffer;
	}

	bool Link( C::ProgramComponent& Program )
	{
		GLint LinkStatus;

		if( Program.bIsLinked )
		{
			//Program is already linked
			return true;
		}

		//Create a program object in OpenGL if we need a new one
		if( Program._ProgramID == 0 )
		{
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
		for( C::ShaderComponent* Shader : Program.LinkedShaders )
		{
			if( Shader && GL::Compile( *Shader ) )
			{
				glAttachShader( Program._ProgramID, Shader->_ShaderID );
			}
		}

		//Add attribute location information to the program
		BindAttributeNames( Program._ProgramID );

		//Do the actual linking
		glLinkProgram( Program._ProgramID );

		//Determing if the linking was successful (saving result to component)
		glGetProgramiv( Program._ProgramID, GL_LINK_STATUS, &LinkStatus );

		if( ( Program.bIsLinked = EGLBool::FromGlobal( LinkStatus ) == EGLBool::True ) )
		{
			GetUniforms( Program._ProgramID, Program._Uniforms );
		}
		else
		{
			DescribeLinkingErrors( std::cerr, Program );
		}

		//Detach shaders before returning success result
		for( C::ShaderComponent* Shader : Program.LinkedShaders )
		{
			if( Shader && Shader->bIsCompiled )
			{
				glDetachShader( Program._ProgramID, Shader->_ShaderID );
				//In the future, this can be delayed until all programs are linked
				glDeleteShader( Shader->_ShaderID );
			}
		}

		return Program.bIsLinked;
	}

	void DescribeLinkingErrors( ostream& Stream, const C::ProgramComponent& Program )
	{
		GLint MsgLength = 0;
		char* MsgBuffer;
		glGetProgramiv( Program._ProgramID, GL_INFO_LOG_LENGTH, &MsgLength );
		MsgBuffer = new char[MsgLength];
		glGetProgramInfoLog( Program._ProgramID, MsgLength, &MsgLength, MsgBuffer );

		Stream << "ERROR [OpenGL] Linking Program " << Program._ProgramID << ": " << MsgBuffer << endl;
		delete[] MsgBuffer;
	}

	void Use( const C::ProgramComponent& Program )
	{
		glUseProgram( Program._ProgramID );
	}
}
