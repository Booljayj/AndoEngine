//
//  GLShader.cpp
//  AndoEngine
//
//  Created by Justin Bool on 9/5/17.
//
//

#include "GLShader.h"
#include <vector>

#include "GLBasicEnums.h"


namespace GL
{
	DefineEnumerationConverter( EShader );
	
	GLuint LoadSource( const char* Source, const EShader::ENUM& Type )
	{
		const GLuint ShaderID = glCreateShader( EShader::ToGlobal( Type ) );
		glShaderSource( ShaderID, 1, &Source, 0 );
		return ShaderID;
	}

	bool Compile( const GLuint& ShaderID )
	{
		GLint CompileStatus;
		glCompileShader( ShaderID );
		glGetShaderiv( ShaderID, GL_COMPILE_STATUS, &CompileStatus );

		return EGLBool::FromGlobal( CompileStatus ) == EGLBool::True;
	}

	void DescribeCompilationErrors( ostream& Stream, const GLuint& ShaderID )
	{
		GLint MsgLength = 0;
		char* MsgBuffer;
		glGetShaderiv( ShaderID, GL_INFO_LOG_LENGTH, &MsgLength );
		MsgBuffer = new char[MsgLength];
		glGetShaderInfoLog( ShaderID, MsgLength, &MsgLength, MsgBuffer );

		Stream << "ERROR [OpenGL] Compiling Shader " << ShaderID << ": " << MsgBuffer << endl;
		delete[] MsgBuffer;
	}

	void Include( const GLuint& ProgramID, const GLuint& ShaderID )
	{
		glAttachShader( ProgramID, ShaderID );
	}

	bool Link( const GLuint& ProgramID )
	{
		GLint LinkStatus;
		glLinkProgram( ProgramID );
		glGetProgramiv( ProgramID, GL_LINK_STATUS, &LinkStatus );

		return EGLBool::FromGlobal( LinkStatus ) == EGLBool::True;
	}

	void DescribeLinkingErrors( ostream& Stream, const GLuint& ProgramID )
	{
		GLint MsgLength = 0;
		char* MsgBuffer;
		glGetProgramiv( ProgramID, GL_INFO_LOG_LENGTH, &MsgLength );
		MsgBuffer = new char[MsgLength];
		glGetProgramInfoLog( ProgramID, MsgLength, &MsgLength, MsgBuffer );

		Stream << "ERROR [OpenGL] Linking Program " << ProgramID << ": " << MsgBuffer << endl;
		delete[] MsgBuffer;
	}
}
