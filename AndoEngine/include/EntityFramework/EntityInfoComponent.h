#pragma once

#include <string>
using namespace std;

#include "EntityFramework/ComponentInfo.h"

namespace C
{
	constexpr const ComponentTypeID EntityInfoComponent_ID = 0;

	struct EntityInfoComponent
	{
		EntityID id;
		string name;
		uint32_t flags;
	};
}
