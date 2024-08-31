#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/Reflection.h"

struct ReflectedType {
	REFLECT_STRUCT(ReflectedType, void);

	ReflectedType() = default;
	ReflectedType(const ReflectedType&) = default;
	virtual ~ReflectedType() = default;

	bool operator==(const ReflectedType& Other) const { return IntegerValue == Other.IntegerValue; };
	bool operator!=(const ReflectedType& Other) const { return !this->operator==(Other); };

	int32_t IntegerValue = 1234;
private:
	bool BooleanValue = true;
};
REFLECT(ReflectedType, Struct);
DEFINE_REFLECTED_SERIALIZATION(ReflectedType);

struct SecondReflectedType : public ReflectedType {
	REFLECT_STRUCT(SecondReflectedType, ReflectedType);
	virtual ~SecondReflectedType() = default;

	std::vector<int32_t> VectorValue;
};
REFLECT(SecondReflectedType, Struct);
DEFINE_REFLECTED_SERIALIZATION(SecondReflectedType);

/*
struct RecursiveType {
	REFLECT_STRUCT( RecursiveType, void );

	size_t Data;
	std::list<std::array<RecursiveType,5>> Nodes;
};
REFLECT( RecursiveType );

struct SerializedTypeA {
	REFLECT_STRUCT( SerializedTypeA, void );

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
	REFLECT_STRUCT( SerializedTypeB, void );

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
