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


struct SerializedTypeA {
	REFLECT();

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

struct SerializedTypeB {
	REFLECT();

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