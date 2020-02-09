#pragma once
#include <cstdint>
#include <ostream>
#include <GL/glew.h>
#include "Rendering/EAttribute.enum.h"
#include "Rendering/EAttributeParameter.enum.h"
#include "Rendering/EBuffer.enum.h"
#include "Rendering/Types.h"

namespace GL {
	void DescribeAttributeParam_Bound(std::ostream& stream, EAttribute::ENUM attribute, EAttributeParameter::ENUM param);
	void DescribeAttribute_Bound(std::ostream& stream, EAttribute::ENUM attribute);

	void DescribeVertexArrayObject(std::ostream& stream, VertexArrayObjectID vaoID);

	void BindAttributeNames(ProgramID pID);
	void BindBuffersToVertexArrayObject(VertexArrayObjectID vaoID, VertexBufferObjectID const* bufferIDArray);
}
