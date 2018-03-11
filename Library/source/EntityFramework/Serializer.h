#pragma once
#include "EntityFramework/Types.h"

template<typename TTData>
struct Serializer
{
	static void Save( TTData const& Comp, ByteStream& Data ) {}
	static void Load( TTData& Comp, ByteStream const& Data ) {}
};
