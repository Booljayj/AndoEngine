#include <iostream>
#include <memory>
#include "Reflection/ReflectionTest.h"
#include "Reflection/Resolver.h"
#include "Reflection/ObjectTypeInfo.h"
#include "Reflection/VariableInfo.h"
#include "Reflection/ConstantInfo.h"
#include "Engine/MemoryView.h"

int16_t ReflectedType::StaticShortValue = 4;
const uint16_t ReflectedType::StaticImmutableShortValue = 15;

#define MEMBER_CONST( __NAME__, __DESC__ )\
new Reflection::TMemberConstantInfo<T, decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )
#define STATIC_CONST( __NAME__, __DESC__ )\
new Reflection::TStaticConstantInfo<decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )
#define MEMBER_VAR( __NAME__, __DESC__ )\
new Reflection::TMemberVariableInfo<T, decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )
#define STATIC_VAR( __NAME__, __DESC__ )\
new Reflection::TStaticVariableInfo<decltype(T::__NAME__)>( #__NAME__, #__DESC__, &T::__NAME__ )

Reflection::ObjectTypeInfo TypeInfo__ReflectedType{
	[]( Reflection::TypeInfo* Info ) {
		using T = ReflectedType;
		Info->Name = "ReflectedType";
		Info->Description = "A simple struct to test reflection";
		Info->Size = sizeof( T );

		if( Reflection::ObjectTypeInfo* ObjectInfo = Info->As<Reflection::ObjectTypeInfo>() ) {
			ObjectInfo->StaticConstants.emplace_back( STATIC_CONST( StaticImmutableShortValue, Test Static Immutable Short Value ) );
			ObjectInfo->MemberConstants.emplace_back( MEMBER_CONST( ImmutableByteValue, Test Immutable Byte Value ) );
			ObjectInfo->StaticVariables.emplace_back( STATIC_VAR( StaticShortValue, Test Static Short Value ) );
			ObjectInfo->MemberVariables.emplace_back( MEMBER_VAR( IntegerValue, Test Integer Value ) );
			ObjectInfo->MemberVariables.emplace_back( MEMBER_VAR( BooleanValue, Test Boolean Value ) );
		}
	}
};

Reflection::TypeInfo* ReflectedType::GetTypeInfo()
{
	return &TypeInfo__ReflectedType;
}
