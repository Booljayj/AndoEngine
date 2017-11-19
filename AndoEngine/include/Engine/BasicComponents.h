#pragma once

#include <vector>

#include "glm/mat4x4.hpp"

namespace C
{
	using namespace glm;
	using namespace std;

	struct TransformComponent
	{
		mat4x4 LocalTransform;
		mat4x4 WorldTransform;
	};

	struct HierarchyComponent
	{
		HierarchyComponent* Parent;
		vector<HierarchyComponent*> Children;
	};
}
