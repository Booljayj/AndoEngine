#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include "EntityFramework/Managers/SimpleComponentManager.h"

/** Allows an entity to exist at a point in space, often relative to another entity */
struct TransformComponent {
	/** The position of the matrix for this component in a transform hierarchy */
	std::tuple<size_t, size_t> TransformHierarchyID;
};

// see https://youtu.be/W45-fsnPhJY?list=WL&t=811 for information on this
//@todo Should this be templatized so other hierarchies can exist? Probably. Call it "ManagedHierarchy".
/** Contains a full hierarchy of transforms in forward-iteration order */
struct TransformHierarchy {
	struct HierarchyEntry {
		bool IsDirty; //True if the local matrix has been modified and the world matrix needs to be updated
		glm::mat4x4 LocalMatrix; //Updated whenever the object itself is changed
		glm::mat4x4 WorldMatrix; //Updated whenever the hierarchy is re-evaluated
	};

	bool bShouldRebuild; //If true, we need to recreate the array of matrices because the hierarchy has changed. This is slow!
	bool bIsDirty; //If true, we need to iterate through the matrices and recalculate the dirty ones
	TransformComponent* Root; //The transform at the root of the hierarchy (it does not have a parent)
	std::vector<HierarchyEntry> Entries; //Entries in forward-iteration order starting with the root, parents will always appear before their children */
};

using TransformComponentManager = TSimpleComponentManager<TransformComponent>;

/** Allows an entity to form a heirarchy with other entities */
struct HierarchyComponent
{
	HierarchyComponent* Parent;
	std::vector<HierarchyComponent*> Children;

	//Event<HierarchyComponent*, HierarchyComponent*> HierarchyChanged;
};

using HierarchyComponentManager = TSimpleComponentManager<HierarchyComponent>;
