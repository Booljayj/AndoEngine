#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <map>
#include <string>
#include <list>
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Resolver/TypeResolver.h"

struct ReflectedType {
	REFLECTION_MEMBERS( ReflectedType );

	ReflectedType() = default;
	ReflectedType( const ReflectedType& Other );
	ReflectedType& operator=( const ReflectedType& Other );

	int32_t IntegerValue = 1234;
	uint8_t const ImmutableByteValue = 3;
	bool BooleanValue = true;

	static int16_t StaticShortValue;
	static const uint16_t StaticImmutableShortValue;
};
REFLECT( ReflectedType );

struct SecondReflectedType {
	REFLECTION_MEMBERS( SecondReflectedType );

	std::vector<ReflectedType> VectorValue;
	std::map<uint8_t, std::string> MapValue;
};
REFLECT( SecondReflectedType );

struct RecursiveType {
	REFLECTION_MEMBERS( RecursiveType );

	size_t Data;
	std::list<std::array<RecursiveType,5>> Nodes;
};
REFLECT( RecursiveType );


struct SerializedTypeA {
	REFLECTION_MEMBERS( SerializedTypeA );

	char CharValue = 'a';
	int8_t ByteValue = 12;
	int16_t ShortValue = 123;
	int32_t IntegerValue = 1234;

	bool BooleanValue = true;

	float FloatValue = 0.1234f;
	double DoubleValue = 1234567.8;

	inline bool operator==( const SerializedTypeA& Other ) const {
		return !(CharValue != Other.CharValue ||
			ByteValue != Other.ByteValue ||
			ShortValue != Other.ShortValue ||
			IntegerValue != Other.IntegerValue ||
			BooleanValue != Other.BooleanValue ||
			FloatValue != Other.FloatValue ||
			DoubleValue != Other.DoubleValue);
	};
};
REFLECT( SerializedTypeA );

struct SerializedTypeB {
	REFLECTION_MEMBERS( SerializedTypeB );

	char CharValue = 'b';
	//int8_t ByteValue = 23;
	int16_t ShortValue = 234;
	//int32_t IntegerValue = 2345;

	bool BooleanValue = false;

	//float FloatValue = 1.234f;
	double DoubleValue = 123456.78;

	inline bool operator==( const SerializedTypeA& Other ) const {
		return !(CharValue != Other.CharValue ||
			ShortValue != Other.ShortValue ||
			BooleanValue != Other.BooleanValue ||
			DoubleValue != Other.DoubleValue);
	};
};
REFLECT( SerializedTypeB );
