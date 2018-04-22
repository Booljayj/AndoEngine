#pragma once
#include "Reflection/TypeInfo.h"

namespace Reflection {
	//@todo Flesh this out more, make it play nice with the existing enum reflection stuff.
	//Enums are a type that describes a set of constant values for the underlying type. Note that they DO NOT have to be numbers,
	// but they will all have a name, and index, and a value.
	struct EnumerationTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Enumeration;

		EnumerationTypeInfo() = delete;
		EnumerationTypeInfo( void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InInitializer, CLASSIFICATION )
		{}
		virtual ~EnumerationTypeInfo() {}

		TypeInfo const* UnderlyingType;
	};
}