//
//  TestComponents.h
//  AndoEngine
//
//  Created by Justin Bool on 7/8/17.
//
//

#pragma once

#include "EntitySystem/General.h"
#include "glm/mat4x4.hpp"

using namespace glm;

namespace C
{
	struct Transform
	{
		mat4x4 LocalTransform;
		mat4x4 WorldTransform;

		void OnRetained() {}
		void OnReleased() {}
	};

	struct Hierarchy
	{
		EntityID Parent;
		vector<EntityID> Children;

		Transform* ParentTransform;
		vector<Transform*> ChildrenTransforms;

		void OnRetained() {}
		void OnReleased() {}
	};
}
