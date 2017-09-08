//
//  GLVertex.h
//  AndoEngine
//
//  Created by Justin Bool on 9/3/17.
//
//

#pragma once

#include "glm/vec3.hpp"

namespace GL
{
	struct VertexData
	{
		glm::vec3 Position;

		VertexData( const float& x, const float& y, const float& z )
		: Position( x, y, z )
		{}
	};
}
