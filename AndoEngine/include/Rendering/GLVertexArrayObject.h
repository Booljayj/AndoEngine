#pragma once
#include <cstdint>
#include <ostream>
#include <GL/glew.h>
#include "Rendering/Attribute.enum.h"
#include "Rendering/AttributeParameter.enum.h"
#include "Rendering/Buffer.enum.h"
#include "Rendering/Types.h"

namespace GL
{
	void DescribeAttributeParam_Bound( std::ostream& Stream, EAttribute::ENUM Attribute, EAttributeParameter::ENUM Param, size_t& Storage );
	void DescribeAttribute_Bound( std::ostream& Stream, EAttribute::ENUM Attribute );

	void DescribeVertexArrayObject( std::ostream& Stream, VertexArrayObjectID VAOID );

	void BindAttributeNames( ProgramID PID );
	void BindBuffersToVertexArrayObject( VertexArrayObjectID VAOID, const VertexBufferObjectID BufferIDArray[EBuffer::Count()] );
}
