#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Engine/STL.h"
#include "Rendering/EGLType.enum.h"

namespace GL {
	struct UniformInfo {
		std::string nameID; //@todo: Change to some kind of 32b string id, only used to identify a particular uniform by a readable name
		EGLType::ENUM type; // is this irrelevant?
		uint32_t location;

		uint8_t componentCount; //the number of 32bit components that compose this type
		uint16_t elementCount; //if uniform is an array, this is the size of the array
	};

	using UniformData = uint32_t;

	void GetUniforms(GLuint programID, std::vector<UniformInfo>& outUniforms);

	template<typename ValueType>
	inline void SetUniform_Bound(GLuint programID, char const* uniformName, ValueType const& value) {
		GLint const location = glGetUniformLocation(programID, uniformName);
		if (location != -1) SetUniformLocation(location, value);
	}

	template<typename ValueType>
	inline void SetUniformLocation_Bound(GLint uniformLocation, ValueType const& value) {}
}

template<>
inline void GL::SetUniformLocation_Bound(GLint uniformLocation, glm::mat4x4 const& value) {
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
}
