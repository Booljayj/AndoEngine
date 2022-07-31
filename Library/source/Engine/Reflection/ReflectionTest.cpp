#include "Engine/Reflection/ReflectionTest.h"
#include "Engine/Reflection.h"

ReflectedType defaults_ReflectedType;
DEFINE_STRUCT_REFLECTION_MEMBERS(,ReflectedType)
	.Description("Reflection test type")
	.Defaults(&defaults_ReflectedType)
	.Variables({
		REFLECT_MVAR(ReflectedType, IntegerValue, ""),
		REFLECT_MVAR(ReflectedType, BooleanValue, "")
	});

DEFINE_STRUCT_REFLECTION_MEMBERS(,SecondReflectedType)
	.Description("Reflection test type")
	.BaseType<ReflectedType>()
	.Variables({
		REFLECT_MVAR(SecondReflectedType, VectorValue, "")
	});

/*
int16_t ReflectedType::StaticShortValue = 4;
uint16_t const ReflectedType::StaticImmutableShortValue = 15;

ReflectedType::ReflectedType( const ReflectedType& Other )
: IntegerValue( Other.IntegerValue )
, ImmutableByteValue( Other.ImmutableByteValue )
, BooleanValue( Other.BooleanValue )
{}

ReflectedType& ReflectedType::operator=( const ReflectedType& Other ) {
	IntegerValue = Other.IntegerValue;
	BooleanValue = Other.BooleanValue;
	return *this;
}

REFLECTED_STRUCT_BEGIN( ReflectedType )
	REFLECT_STATIC_CONSTANT( StaticImmutableShortValue, "An immutable static short" );
	DEFINE_STATIC_CONSTANT_FIELDS( &StaticImmutableShortValue );

	REFLECT_MEMBER_CONSTANT( ImmutableByteValue, "An immutable byte value" );
	DEFINE_MEMBER_CONSTANT_FIELDS( &ImmutableByteValue );

	REFLECT_STATIC_VARIABLE( StaticShortValue, "A static short" );
	DEFINE_STATIC_VARIABLE_FIELDS( &StaticShortValue );

	REFLECT_MEMBER_VARIABLE( IntegerValue, "An integer value" );
	REFLECT_MEMBER_VARIABLE( BooleanValue, "A boolean value" );
	DEFINE_MEMBER_VARIABLE_FIELDS( &IntegerValue, &BooleanValue );
REFLECTED_STRUCT_END()
DEFINE_DECLARE_STRUCT_REFLECTION_MEMBERS( ReflectedType, "A simple struct to test reflection" );
*/

/*
STRUCT_TYPE_BEGIN( ReflectedType )
{
	StructInfo->Description = "A simple struct to test reflection";
	MAKE_DEFAULT();
	ADD_STATIC_CONSTANT( StaticImmutableShortValue, "Test Static Immutable Short Value" );
	ADD_MEMBER_CONSTANT( ImmutableByteValue, "Test Immutable Byte Value" );
	ADD_STATIC_VARIABLE( StaticShortValue, "Test Static Short Value" );
	ADD_MEMBER_VARIABLE( IntegerValue, "Test Integer Value" );
	ADD_MEMBER_VARIABLE( BooleanValue, "Test Boolean Value" );
}
STRUCT_TYPE_END( ReflectedType )

STRUCT_TYPE_BEGIN( SecondReflectedType )
{
	StructInfo->Description = "Another struct to test reflection, in particular nesting";
	MAKE_DEFAULT();
	ADD_MEMBER_VARIABLE( VectorValue, "A vector that contains another reflected type" );
	ADD_MEMBER_VARIABLE( MapValue, "A map that contains strings" );
}
STRUCT_TYPE_END( SecondReflectedType )

STRUCT_TYPE_BEGIN( RecursiveType )
{
	StructInfo->Description = "Recursive struct example";
	MAKE_DEFAULT();
	ADD_MEMBER_VARIABLE( Data, "Some data" );
	ADD_MEMBER_VARIABLE( Nodes, "Recursive list of the same type" );
}
STRUCT_TYPE_END( RecursiveType )

STRUCT_TYPE_BEGIN( SerializedTypeA )
{
	StructInfo->Description = "Serialized type example A";
	MAKE_DEFAULT();
	ADD_MEMBER_VARIABLE( CharValue, "" );
	ADD_MEMBER_VARIABLE( ByteValue, "" );
	ADD_MEMBER_VARIABLE( ShortValue, "" );
	ADD_MEMBER_VARIABLE( IntegerValue, "" );
	ADD_MEMBER_VARIABLE( BooleanValue, "" );
	ADD_MEMBER_VARIABLE( FloatValue, "" );
	ADD_MEMBER_VARIABLE( DoubleValue, "" );
	MAKE_SERIALIZER();
}
STRUCT_TYPE_END( SerializedTypeA )

STRUCT_TYPE_BEGIN( SerializedTypeB )
{
	StructInfo->Description = "Serialized type example B";
	MAKE_DEFAULT();
	ADD_MEMBER_VARIABLE( CharValue, "" );
	ADD_MEMBER_VARIABLE( ShortValue, "" );
	ADD_MEMBER_VARIABLE( BooleanValue, "" );
	ADD_MEMBER_VARIABLE( DoubleValue, "" );
	MAKE_SERIALIZER();
}
STRUCT_TYPE_END( SerializedTypeB )
*/
