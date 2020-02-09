#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace GL {
	//Vertex data, packed into 8 bytes
	struct VertexData {
		glm::vec3 position;
		glm::bvec4 texCoords; //should actually be a ushort vec2

		glm::vec3 normal;
		glm::bvec4 color;

		VertexData(float x, float y, float z)
		: position(x, y, z)
		, texCoords(0, 0, 0, 0)
		, normal(1, 0, 0)
		, color(1, 0, 0, 1)
		{}

		VertexData(glm::vec3 const& inPosition)
		: position(inPosition)
		, texCoords(0, 0, 0, 0)
		, normal(1, 0, 0)
		, color(1, 0, 0, 1)
		{}
	};
}
