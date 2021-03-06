#pragma once
#include "Engine/STL.h"
#include "Reflection/StructTypeInfo.h"

struct ReflectedType {
	REFLECTION_MEMBERS(ReflectedType, void);

	ReflectedType() = default;
	ReflectedType(const ReflectedType&) = default;
	bool operator==(const ReflectedType& Other) const { return IntegerValue == Other.IntegerValue; };
	bool operator!=(const ReflectedType& Other) const { return !this->operator==(Other); };

	int32_t IntegerValue = 1234;
	bool BooleanValue = true;
};
REFLECT(ReflectedType);

struct SecondReflectedType : public ReflectedType {
	REFLECTION_MEMBERS(SecondReflectedType, void);

	std::vector<int32_t> VectorValue;
};
REFLECT(SecondReflectedType);

/*
struct RecursiveType {
	REFLECTION_MEMBERS( RecursiveType, void );

	size_t Data;
	std::list<std::array<RecursiveType,5>> Nodes;
};
REFLECT( RecursiveType );

struct SerializedTypeA {
	REFLECTION_MEMBERS( SerializedTypeA, void );

	char CharValue = 'a';
	int8_t ByteValue = 12;
	int16_t ShortValue = 123;
	int32_t IntegerValue = 1234;

	bool BooleanValue = true;

	float FloatValue = 0.1234f;
	double DoubleValue = 1234567.8;

	inline bool operator==( const SerializedTypeA& Other ) const {
		return (CharValue == Other.CharValue &&
			ByteValue == Other.ByteValue &&
			ShortValue == Other.ShortValue &&
			IntegerValue == Other.IntegerValue &&
			BooleanValue == Other.BooleanValue &&
			FloatValue == Other.FloatValue &&
			DoubleValue == Other.DoubleValue);
	};
};
REFLECT( SerializedTypeA );

struct SerializedTypeB {
	REFLECTION_MEMBERS( SerializedTypeB, void );

	char CharValue = 'b';
	//int8_t ByteValue = 23;
	int16_t ShortValue = 234;
	//int32_t IntegerValue = 2345;

	bool BooleanValue = false;

	//float FloatValue = 1.234f;
	double DoubleValue = 123456.78;

	inline bool operator==( const SerializedTypeA& Other ) const {
		return (CharValue == Other.CharValue &&
			ShortValue == Other.ShortValue &&
			BooleanValue == Other.BooleanValue &&
			DoubleValue == Other.DoubleValue);
	};
};
REFLECT( SerializedTypeB );
*/
