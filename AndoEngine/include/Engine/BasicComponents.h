#pragma once
#include <vector>
#include "glm/mat4x4.hpp"

namespace C
{
	struct TransformComponent
	{
		glm::mat4x4 LocalTransform;
		glm::mat4x4 WorldTransform;
	};

	struct HierarchyComponent
	{
		HierarchyComponent* Parent;
		std::vector<HierarchyComponent*> Children;
	};
}
