#include <iostream>
#include <memory>
#include "Reflection/ReflectionTest.h"
#include "Reflection/Resolver.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/StructTypeMacros.h"
#include "Reflection/Components/VariableInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Serialization/StructSerializer.h"

int16_t ReflectedType::StaticShortValue = 4;
uint16_t const ReflectedType::StaticImmutableShortValue = 15;

ReflectedType::ReflectedType( const ReflectedType& Other )
: IntegerValue( Other.IntegerValue )
, ImmutableByteValue( Other.ImmutableByteValue )
, BooleanValue( Other.BooleanValue )
{}

ReflectedType& ReflectedType::operator=( const ReflectedType& Other )
{
	IntegerValue = Other.IntegerValue;
	BooleanValue = Other.BooleanValue;
	return *this;
}

STRUCT_TYPE_BEGIN( ReflectedType ) {
	StructInfo->Description = "A simple struct to test reflection";
	MAKE_DEFAULT();
	ADD_STATIC_CONSTANT( StaticImmutableShortValue, "Test Static Immutable Short Value" );
	ADD_MEMBER_CONSTANT( ImmutableByteValue, "Test Immutable Byte Value" );
	ADD_STATIC_VARIABLE( StaticShortValue, "Test Static Short Value" );
	ADD_MEMBER_VARIABLE( IntegerValue, "Test Integer Value" );
	ADD_MEMBER_VARIABLE( BooleanValue, "Test Boolean Value" );
}
STRUCT_TYPE_END( ReflectedType, "Reflection::ReflectedType" )

STRUCT_TYPE_BEGIN( SecondReflectedType ) {
	StructInfo->Description = "Another struct to test reflection, in particular nesting";
	MAKE_DEFAULT();
	ADD_MEMBER_VARIABLE( VectorValue, "A vector that contains another reflected type" );
	ADD_MEMBER_VARIABLE( MapValue, "A map that contains strings" );
}
STRUCT_TYPE_END( SecondReflectedType, "Reflection::SecondReflectedType" )

STRUCT_TYPE_BEGIN( RecursiveType ) {
	StructInfo->Description = "Recursive struct example";
	MAKE_DEFAULT();
	ADD_MEMBER_VARIABLE( Data, "Some data" );
	ADD_MEMBER_VARIABLE( Nodes, "Recursive list of the same type" );
}
STRUCT_TYPE_END( RecursiveType, "Reflection::RecursiveType" )

STRUCT_TYPE_BEGIN( SerializedTypeA ) {
	StructInfo->Description = "Serialized type example A";
	MAKE_DEFAULT();
	MAKE_SERIALIZER();
	ADD_MEMBER_VARIABLE( CharValue, "" );
	ADD_MEMBER_VARIABLE( ByteValue, "" );
	ADD_MEMBER_VARIABLE( ShortValue, "" );
	ADD_MEMBER_VARIABLE( IntegerValue, "" );
	ADD_MEMBER_VARIABLE( BooleanValue, "" );
	ADD_MEMBER_VARIABLE( FloatValue, "" );
	ADD_MEMBER_VARIABLE( DoubleValue, "" );
}
STRUCT_TYPE_END( SerializedTypeA, "Reflection::SerializedTypeB" )

STRUCT_TYPE_BEGIN( SerializedTypeB ) {
	StructInfo->Description = "Serialized type example B";
	MAKE_DEFAULT();
	MAKE_SERIALIZER();
	ADD_MEMBER_VARIABLE( CharValue, "" );
	ADD_MEMBER_VARIABLE( ShortValue, "" );
	ADD_MEMBER_VARIABLE( BooleanValue, "" );
	ADD_MEMBER_VARIABLE( DoubleValue, "" );
}
STRUCT_TYPE_END( SerializedTypeB, "Reflection::SerializedTypeB" )
