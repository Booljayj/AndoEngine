#pragma once
#include "Engine/Logging/LogCategory.h"
#include "EntityFramework/Types.h"

DECLARE_LOG_CATEGORY(Entity);

/** An Entity is an identifiable object in the game. It can own components, which define different sets of data */
struct Entity {
public:
	Entity() = default;
	Entity(Entity&& other) = default;
	Entity(const Entity& other) = delete;

	Entity& operator=(Entity&& other) = default;
	Entity& operator=(const Entity& other) = delete;
};
