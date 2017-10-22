//
//  GLVertexBufferObject.hpp
//  AndoEngine
//
//  Created by Justin Bool on 9/3/17.
//
//

#pragma once

#include <cstdint>
#include <GL/glew.h>

#include "AndoEngine/EnumMacros.h"

namespace GL
{
	DeclareEnumeration(
		EBuffer,
		uint8_t,
		(
			VertexData,
			InstanceData
		)
	);
}
