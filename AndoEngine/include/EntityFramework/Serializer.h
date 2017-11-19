#pragma once

#include "EntityFramework/Types.h"

template<typename TTData>
struct Serializer
{
	static void Save( const TTData& comp, ByteStream& bytes ) {}
	static void Load( TTData& comp, const ByteStream& bytes ) {}
};
