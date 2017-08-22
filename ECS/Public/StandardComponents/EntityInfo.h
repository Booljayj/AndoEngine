//
//  EntityInfo.h
//  ECS
//
//  Created by Justin Bool on 6/18/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include <string>
using namespace std;

#include "TCompInfo.h"
#include "TCompManagerChunked.h"

namespace C
{
	struct EntityInfo
	{
		EntityID id;
		string name;
		uint32_t flags;
	};
}

TCompManagerChunked<EntityInfo> EntityInfoManager{ 100 };
const TCompInfo<C::EntityInfo> EntityInfo{ 0, "EntityInfo", &EntityInfoManager };
