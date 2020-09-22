#include "Rendering/EGLBool.enum.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexArrayObject.h"

namespace GL {
	bool Compile(ShaderComponent& shader) {
		if (shader.isCompiled) {
			//shader is already compiled.
			return true;
		}

		//Create a new OpenGL shader object if we need one.
		if (shader.shaderID == 0) {
			shader.shaderID = glCreateShader(EShader::ToGL(shader.shaderType));
		}
		glShaderSource(shader.shaderID, 1, &shader.source, 0);

		GLint compileStatus;
		glCompileShader(shader.shaderID);
		glGetShaderiv(shader.shaderID, GL_COMPILE_STATUS, &compileStatus);

		if (!(shader.isCompiled = EGLBool::FromGL(compileStatus) == EGLBool::True)) {
			DescribeCompilationErrors(std::cerr, shader);
		}

		return shader.isCompiled;
	}

	void DescribeCompilationErrors(std::ostream& stream, ShaderComponent const& shader) {
		GLint msgLength = 0;
		glGetShaderiv(shader.shaderID, GL_INFO_LOG_LENGTH, &msgLength);
		char* const msgBuffer = new char[msgLength];
		glGetShaderInfoLog(shader.shaderID, msgLength, &msgLength, msgBuffer);

		stream << "ERROR [OpenGL] Compiling shader " << shader.shaderID << ": " << msgBuffer << std::endl;
		delete[] msgBuffer;
	}

	bool Link(ProgramComponent& program) {

		if( program.isLinked ) {
			//program is already linked
			return true;
		}

		//Create a program object in OpenGL if we need a new one
		if( program.programID == 0 ) {
			program.programID = glCreateProgram();
		}

		//Attempt to load precompiled program data
//		if (program._Binary) {
//			glProgramBinary(program.programID, program._BinaryVersion, program._Binary.size(), program._Binary.data());
//			glGetProgramiv(program.programID, GL_LINK_STATUS, &linkStatus);
//			linkingWasSuccessful = EGLBool::FromGlobal(linkStatus) == EGLBool::True;
//
//			if (linkingWasSuccessful) {
//				//The program was loaded from an existing binary. Huzzah.
//				return true;
//			}
//		}

		//Compile and attach any shaders this program depends on.
		for (ShaderComponent* shader : program.linkedShaders) {
			if(shader && GL::Compile(*shader)) {
				glAttachShader(program.programID, shader->shaderID);
			}
		}

		//Add attribute location information to the program
		BindAttributeNames(program.programID);

		//Do the actual linking
		glLinkProgram(program.programID);

		//Determing if the linking was successful (saving result to component)
		GLint linkStatus;
		glGetProgramiv(program.programID, GL_LINK_STATUS, &linkStatus);

		if ((program.isLinked = EGLBool::FromGL(linkStatus) == EGLBool::True)) {
			GetUniforms(program.programID, program.uniforms);
		} else {
			DescribeLinkingErrors(std::cerr, program);
		}

		//Detach shaders before returning success result
		for (ShaderComponent* shader : program.linkedShaders) {
			if (shader && shader->isCompiled) {
				glDetachShader(program.programID, shader->shaderID);
				//In the future, this can be delayed until all programs are linked
				glDeleteShader(shader->shaderID);
			}
		}

		return program.isLinked;
	}

	void DescribeLinkingErrors(std::ostream& stream, ProgramComponent const& program) {
		GLint msgLength = 0;
		glGetProgramiv(program.programID, GL_INFO_LOG_LENGTH, &msgLength);
		char* const msgBuffer = new char[msgLength];
		glGetProgramInfoLog(program.programID, msgLength, &msgLength, msgBuffer);

		stream << "ERROR [OpenGL] Linking program " << program.programID << ": " << msgBuffer << std::endl;
		delete[] msgBuffer;
	}

	void Use(ProgramComponent const& program) {
		glUseProgram(program.programID);
	}
}
