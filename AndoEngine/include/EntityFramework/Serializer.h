// Copyright © 2017 Justin Bool. All rights reserved.

#pragma once

#include "EntityFrameworkTypes.h"

template<typename TTData>
struct Serializer
{
	static void Save( const TTData& comp, ByteStream& bytes ) {}
	static void Load( TTData& comp, const ByteStream& bytes ) {}
};
