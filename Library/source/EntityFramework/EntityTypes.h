#pragma once
#include "Engine/STL.h"
#include "EntityFramework/EntityInternals.h"

/** A long-term identifier for an entity */
using EntityAssetID = uint64_t;
/** A runtime-only identifier for an entity. Not long-term. */
using EntityID = entt::entity;
