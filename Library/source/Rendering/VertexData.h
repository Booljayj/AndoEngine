#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace GL
{
	//Vertex data, packed into 8 bytes
	struct VertexData
	{
		glm::vec3 Position;
		glm::bvec4 TexCoords; //should actually be a ushort vec2

		glm::vec3 Normal;
		glm::bvec4 Color;

		VertexData( float x, float y, float z )
		: Position( x, y, z )
		, TexCoords( 0, 0, 0, 0 )
		, Normal( 1, 0, 0 )
		, Color( 1, 0, 0, 1 )
		{}

		VertexData( glm::vec3 const& InPosition )
		: Position( InPosition )
		, TexCoords( 0, 0, 0, 0 )
		, Normal( 1, 0, 0 )
		, Color( 1, 0, 0, 1 )
		{}
	};
}
