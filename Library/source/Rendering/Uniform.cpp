#include <glm/mat4x4.hpp>
#include "Rendering/Uniform.h"

using namespace std;

namespace GL {
	void GetUniforms(GLuint programID, vector<UniformInfo>& outUniforms) {
		GLint totalCount = 0;
		glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &totalCount);
		GLint nameBufferSize = 0;
		glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &nameBufferSize);

		char* const nameBuffer = new char[nameBufferSize];

		outUniforms.clear();
		outUniforms.reserve(totalCount);

		GLint nameSize = 0;
		GLint elementCount = 0;
		GLenum type = 0;

		for (GLuint uniformIndex = 0; uniformIndex < totalCount; ++ uniformIndex) {
			glGetActiveUniform(programID, uniformIndex, nameBufferSize, &nameSize, &elementCount, &type, nameBuffer);

			UniformInfo newUniformInfo;
			newUniformInfo.nameID = string{nameBuffer, static_cast<string::size_type>(nameSize)};
			newUniformInfo.type = EGLType::FromGL(type);
			newUniformInfo.elementCount = static_cast<uint16_t>(elementCount);
			newUniformInfo.location = static_cast<uint32_t>(glGetUniformLocation(programID, nameBuffer));

			outUniforms.push_back(newUniformInfo);
		}

		delete[] nameBuffer;
	}
}
