// Copyright © 2017 Justin Bool. All rights reserved.

#pragma once

#include <string>
using namespace std;

#include "ComponentInfo.h"

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
