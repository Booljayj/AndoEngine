#pragma once
#include <cstdint>
#include "Reflection/TypeInfo.h"

struct ReflectedType {
	static Reflection::TypeInfo* GetTypeInfo();

	int32_t IntegerValue = 1234;
	uint8_t const ImmutableByteValue = 3;
	bool BooleanValue = true;

	static int16_t StaticShortValue;
	static const uint16_t StaticImmutableShortValue;
};
