#pragma once
#define GLM_FORCE_RADIANS
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtx/exterior_product.hpp>
#include <glm/matrix.hpp>
#include "Engine/Utility.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	namespace Internal {
		DECLARE_RESOLVER(glm::vec2);
		DECLARE_RESOLVER(glm::vec3);
		DECLARE_RESOLVER(glm::vec4);

		DECLARE_RESOLVER(glm::ivec2);
		DECLARE_RESOLVER(glm::ivec3);

		DECLARE_RESOLVER(glm::mat2x2);
		DECLARE_RESOLVER(glm::mat2x3);
		DECLARE_RESOLVER(glm::mat2x4);

		DECLARE_RESOLVER(glm::mat3x2);
		DECLARE_RESOLVER(glm::mat3x3);
		DECLARE_RESOLVER(glm::mat3x4);

		DECLARE_RESOLVER(glm::mat4x2);
		DECLARE_RESOLVER(glm::mat4x3);
		DECLARE_RESOLVER(glm::mat4x4);
	}
}
