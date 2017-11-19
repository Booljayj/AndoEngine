#pragma once
#include <cstdint>
#include <ostream>
#include <string>
#include <GL/glew.h>
#include "Rendering/Attribute.enum.h"
#include "Rendering/AttributeBoolParameter.enum.h"
#include "Rendering/AttributeIntParameter.enum.h"
#include "Rendering/AttributePtrParameter.enum.h"
#include "Rendering/AttributeTypeParameter.enum.h"
#include "Rendering/Buffer.enum.h"
#include "Rendering/GLVertexBufferObject.h"

namespace GL
{
	void DescribeParam_Bound( std::ostream& Stream, EAttribute::ENUM Attribute, EAttributeIntParameter::ENUM Param, size_t& Storage );
	void DescribeParam_Bound( std::ostream& Stream, EAttribute::ENUM Attribute, EAttributeTypeParameter::ENUM Param, size_t& Storage );
	void DescribeParam_Bound( std::ostream& Stream, EAttribute::ENUM Attribute, EAttributeBoolParameter::ENUM Param, size_t& Storage );
	void DescribeParam_Bound( std::ostream& Stream, EAttribute::ENUM Attribute, EAttributePtrParameter::ENUM Param, size_t& Storage );

	std::string DescribeVertexArrayObject( GLuint VAOID );

	void BindAttributeNames( GLuint ProgramID );
	void BindBuffersToVertexArrayObject( GLuint VAOID, const GLuint BufferID[EBuffer::Count()] );
}
