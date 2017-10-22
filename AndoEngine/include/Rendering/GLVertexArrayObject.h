//
//  GLVertexAttributes.hpp
//  AndoEngine
//
//  Created by Justin Bool on 9/3/17.
//
//

#pragma once

#include <cstdint>
#include <ostream>
using namespace std;
#include <GL/glew.h>

#include "AndoEngine/EnumMacros.h"

#include "GLVertexBufferObject.h"

namespace GL
{
	DeclareEnumeration(
		EAttribute,
		uint8_t,
		(
			Position,
			Color,
			Normal,
			TexCoords,
			BoneIndexes,
			BoneWeights
		)
	);

	DeclareEnumerationConverter(
		EAttributeIntParameter,
		( uint8_t, GLenum ),
		(
			( BufferID, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING ),
			( Size, GL_VERTEX_ATTRIB_ARRAY_SIZE ),
			( Stride, GL_VERTEX_ATTRIB_ARRAY_STRIDE ),
			( Integer, GL_VERTEX_ATTRIB_ARRAY_INTEGER ),
			( Divisor, GL_VERTEX_ATTRIB_ARRAY_DIVISOR )
		)
	);

	DeclareEnumerationConverter(
		EAttributeTypeParameter,
		( uint8_t, GLenum ),
		(
			( Type, GL_VERTEX_ATTRIB_ARRAY_TYPE )
		)
	);

	DeclareEnumerationConverter(
		EAttributeBoolParameter,
		( uint8_t, GLenum ),
		(
			( Enabled, GL_VERTEX_ATTRIB_ARRAY_ENABLED ),
			( Normalized, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED )
		)
	);

	DeclareEnumerationConverter(
		EAttributePtrParameter,
		( uint8_t, GLenum ),
		(
			( Offset, GL_VERTEX_ATTRIB_ARRAY_POINTER )
		)
	);

	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributeIntParameter::ENUM Param, size_t& Storage );
	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributeTypeParameter::ENUM Param, size_t& Storage );
	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributeBoolParameter::ENUM Param, size_t& Storage );
	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributePtrParameter::ENUM Param, size_t& Storage );

	string DescribeVertexArrayObject( GLuint VAOID );

	void BindAttributeNames( GLuint ProgramID );
	void BindBuffersToVertexArrayObject( GLuint VAOID, const GLuint BufferID[EBuffer::Count()] );
}
