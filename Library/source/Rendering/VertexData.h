#pragma once
#include "Geometry/LinearAlgebra.h

namespace GL {
	//Vertex data, packed into 8 bytes
	struct VertexData {
		glm::vec3 position;
		glm::vec<2, uint16_t> texCoords;

		glm::vec3 normal;
		glm::vec4 color;

		VertexData() = default;

		VertexData(float x, float y, float z)
		: position(x, y, z)
		, texCoords(0, 0)
		, normal(1, 0, 0)
		, color(1, 0, 0, 1)
		{}

		VertexData(glm::vec3 const& inPosition)
		: position(inPosition)
		, texCoords(0, 0)
		, normal(1, 0, 0)
		, color(1, 0, 0, 1)
		{}
	};
}
