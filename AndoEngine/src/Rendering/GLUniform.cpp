#include <glm/mat4x4.hpp>
#include "Rendering/GLUniform.h"

using namespace std;

namespace GL
{
	void GetUniforms( const GLuint& ProgramID, vector<UniformInfo>& OutUniforms )
	{
		GLint TotalCount = 0;
		glGetProgramiv( ProgramID, GL_ACTIVE_UNIFORMS, &TotalCount );
		GLint NameBufferSize = 0;
		glGetProgramiv( ProgramID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &NameBufferSize );

		char* NameBuffer = new char[NameBufferSize];

		OutUniforms.clear();
		OutUniforms.reserve( TotalCount );

		GLint NameSize = 0;
		GLint ElementCount = 0;
		GLenum Type = 0;

		for( GLuint UniformIndex = 0; UniformIndex < TotalCount; ++ UniformIndex )
		{
			glGetActiveUniform(	ProgramID, UniformIndex, NameBufferSize, &NameSize, &ElementCount, &Type, NameBuffer );

			UniformInfo NewUniformInfo;
			NewUniformInfo.NameID = string{ NameBuffer, static_cast<string::size_type>( NameSize ) };
			NewUniformInfo.Type = EGLType::FromGL( Type );
			NewUniformInfo.ElementCount = static_cast<uint16_t>( ElementCount );
			NewUniformInfo.Location = static_cast<uint32_t>( glGetUniformLocation( ProgramID, NameBuffer ) );

			OutUniforms.push_back( NewUniformInfo );
		}

		delete[] NameBuffer;
	}
}
