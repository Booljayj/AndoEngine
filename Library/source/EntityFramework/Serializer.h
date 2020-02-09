#pragma once
#include "EntityFramework/Types.h"

template<typename DataType>
struct Serializer {
	static void Save(DataType const& Comp, ByteStream& Data) {}
	static void Load(DataType& Comp, ByteStream const& Data) {}
};
