#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include "EntityFramework/Managers/SimpleComponentManager.h"

namespace C
{
	/** Allows an entity to exist at a point in space, often relative to another entity */
	struct TransformComponent
	{
		glm::mat4x4 LocalTransform;
		glm::mat4x4 WorldTransform;
	};

	using TransformComponentManager = TSimpleComponentManager<TransformComponent>;

	/** Allows an entity to form a heirarchy with other entities */
	struct HierarchyComponent
	{
		HierarchyComponent* Parent;
		std::vector<HierarchyComponent*> Children;
	};

	using HierarchyComponentManager = TSimpleComponentManager<HierarchyComponent>;
}
