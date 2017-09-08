//
//  Shader.h
//  AndoEngine
//
//  Created by Justin Bool on 8/6/17.
//
//

#pragma once

#include "GL/glew.h"

namespace C
{
	struct Shader
	{
		const char* Source = nullptr;
		GLenum ShaderType = 0;

		GLuint ShaderID = 0;
		GLint CompileStatus = 0;

		void OnRetained() {}

		void OnReleased()
		{
			if( ShaderID != 0 ) Delete();
		}

		bool IsValid()
		{
			return strlen( Source ) > 1;
		}

		void Compile()
		{
			ShaderID = glCreateShader( ShaderType );
			//Send the shader source to the GPU
			glShaderSource( ShaderID, 1, &Source, 0 );
			//Compile the shader source into an object
			glCompileShader( ShaderID );
			//Get the results of the compilation
			glGetShaderiv( ShaderID, GL_COMPILE_STATUS, &CompileStatus );
		}

		void Delete()
		{
			glDeleteShader( ShaderID );
		}
	};
}
