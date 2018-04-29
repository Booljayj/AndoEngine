#include <iostream>
#include <memory>
#include "Reflection/ReflectionTest.h"
#include "Reflection/Resolver.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Engine/MemoryView.h"

int16_t ReflectedType::StaticShortValue = 4;
const uint16_t ReflectedType::StaticImmutableShortValue = 15;

#define MEMBER_CONST( __NAME__, __DESC__ )\
new TMemberConstantInfo<T, decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )
#define STATIC_CONST( __NAME__, __DESC__ )\
new TStaticConstantInfo<decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )
#define MEMBER_VAR( __NAME__, __DESC__ )\
new TMemberVariableInfo<T, decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )
#define STATIC_VAR( __NAME__, __DESC__ )\
new TStaticVariableInfo<decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )

namespace Reflection {
	namespace {
		using T = ReflectedType;
		StructTypeInfo TypeInfo__ReflectedType{
			"ReflectedType",
			sizeof( T ),
			[]( TypeInfo* Info ) {
				Info->Description = "A simple struct to test reflection";

				if( auto* StructInfo = Info->As<Reflection::StructTypeInfo>() ) {
					StructInfo->StaticConstants.emplace_back( STATIC_CONST( StaticImmutableShortValue, Test Static Immutable Short Value ) );
					StructInfo->MemberConstants.emplace_back( MEMBER_CONST( ImmutableByteValue, Test Immutable Byte Value ) );
					StructInfo->StaticVariables.emplace_back( STATIC_VAR( StaticShortValue, Test Static Short Value ) );
					StructInfo->MemberVariables.emplace_back( MEMBER_VAR( IntegerValue, Test Integer Value ) );
					StructInfo->MemberVariables.emplace_back( MEMBER_VAR( BooleanValue, Test Boolean Value ) );
				}
			}
		};
	}
}

Reflection::TypeInfo* ReflectedType::GetTypeInfo() {
	return &Reflection::TypeInfo__ReflectedType;
}
