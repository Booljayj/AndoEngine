#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Rendering/GLType.enum.h"

namespace GL
{
	struct UniformInfo
	{
		std::string NameID; //@todo: Change to some kind of 32b string id, only used to identify a particular uniform by a readable name
		EGLType::ENUM Type; // is this irrelevant?
		uint32_t Location;

		uint8_t ComponentCount; //the number of 32bit components that compose this type
		uint16_t ElementCount; //if uniform is an array, this is the size of the array
	};

	using UniformData = uint32_t;

	void GetUniforms( const GLuint& ProgramID, std::vector<UniformInfo>& OutUniforms );

	template< typename TValue >
	inline void SetUniform_Bound( const GLuint& ProgramID, const char* UniformName, const TValue& Value )
	{
		const GLint Location = glGetUniformLocation( ProgramID, UniformName );
		if( Location != -1 )
		{
			SetUniformLocation( Location, Value );
		}
	}

	template< typename TValue >
	inline void SetUniformLocation_Bound( const GLint& UniformLocation, const TValue& Value ) {}
}

template<>
inline void GL::SetUniformLocation_Bound( const GLint& UniformLocation, const glm::mat4x4& Value )
{
	glUniformMatrix4fv( UniformLocation, 1, GL_FALSE, glm::value_ptr( Value ) );
}
