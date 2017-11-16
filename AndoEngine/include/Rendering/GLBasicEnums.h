//
//  GLBasicEnums.hpp
//  AndoEngine
//
//  Created by Justin Bool on 9/3/17.
//
//

#pragma once

#include <cstdint>
#include "GL/glew.h"
#include "Rendering/GLType.enum.h"

#include "AndoEngine/EnumMacros.h"

namespace GL
{
	DeclareEnumerationConverter(
		EGLBool,
		( uint8_t, GLenum ),
		(
			( False, GL_FALSE ),
			( True, GL_TRUE )
		)
	);
}
