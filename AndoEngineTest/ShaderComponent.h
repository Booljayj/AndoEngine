//
//  Shader.h
//  AndoEngine
//
//  Created by Justin Bool on 8/6/17.
//
//

#pragma once

namespace C
{
	struct Shader
	{
		const char* Source;
		GLenum ShaderType;

		GLuint ShaderID;
		GLint CompileStatus;

		void OnRetained() {}
		void OnReleased() {}

		bool IsValid()
		{
			return strlen( Source ) > 1;
		}
	};
}
