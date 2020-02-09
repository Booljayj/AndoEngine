#pragma once
#include "EntityFramework/Managers/ChunkedComponentManager.h"

struct Entity;

/** An extremely basic component manager that does not perform any additional lifetime functions other than wiping newly retained components. */
template<typename ComponentType>
struct TSimpleComponentManager : TChunkedComponentManager<ComponentType, 64> {
	using TChunkedComponentManager<ComponentType, 64>::TChunkedComponentManager;
};
