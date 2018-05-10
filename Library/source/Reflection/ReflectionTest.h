#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <map>
#include <string>
#include <list>
#include "Reflection/TypeInfo.h"

struct ReflectedType {
	REFLECT();

	ReflectedType() = default;
	ReflectedType( const ReflectedType& Other );
	ReflectedType& operator=( const ReflectedType& Other );

	int32_t IntegerValue = 1234;
	uint8_t const ImmutableByteValue = 3;
	bool BooleanValue = true;

	static int16_t StaticShortValue;
	static const uint16_t StaticImmutableShortValue;
};

struct SecondReflectedType {
	REFLECT();

	std::vector<ReflectedType> VectorValue;
	std::map<uint8_t, std::string> MapValue;
};

struct RecursiveType {
	REFLECT();

	size_t Data;
	std::list<std::array<RecursiveType,5>> Nodes;
};
