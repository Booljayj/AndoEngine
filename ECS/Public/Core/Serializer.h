//
//  serialize.h
//  ECS
//
//  Created by Justin Bool on 4/12/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include "General.h"

template<typename TTData>
struct Serializer
{
	static void Save( const TTData& comp, ByteStream& bytes ) {}
	static void Load( TTData& comp, const ByteStream& bytes ) {}
};
