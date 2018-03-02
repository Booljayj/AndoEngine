#pragma once
#include <string>
#include "EntityFramework/Types.h"

namespace C
{
	constexpr const ComponentTypeID EntityInfoComponent_ID = 0;

	struct EntityInfoComponent
	{
		EntityID id;
		std::string name;
		uint32_t flags;
	};
}
