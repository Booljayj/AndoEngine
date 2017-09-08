//
//  GLShader.hpp
//  AndoEngine
//
//  Created by Justin Bool on 9/5/17.
//
//

#pragma once

#include <ostream>
using namespace std;

#include "GL/glew.h"

#include "EnumMacros.h"

namespace GL
{
	DeclareEnumerationConverter(
	EShader,
		( uint8_t, GLenum ),
		(
			( Vertex, GL_VERTEX_SHADER ),
			( Fragment, GL_FRAGMENT_SHADER )
		)
	);

	GLuint LoadSource( const char* Source, const EShader::ENUM& Type );
	bool Compile( const GLuint& ShaderID );

	void DescribeCompilationErrors( ostream& Stream, const GLuint& ShaderID );

	void Include( const GLuint& ProgramID, const GLuint& ShaderID );
	bool Link( const GLuint& ProgramID );

	void DescribeLinkingErrors( ostream& Stream, const GLuint& ProgramID );
}
