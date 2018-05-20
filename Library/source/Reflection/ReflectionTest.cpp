#include <iostream>
#include <memory>
#include "Reflection/ReflectionTest.h"
#include "Reflection/Resolver.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/StructTypeMacros.h"
#include "Reflection/Components/VariableInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Serialization/StructSerializer.h"
#include "Engine/MemoryView.h"

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

namespace Reflection {
	STRUCT_TYPE_REFLECT_BEGIN( ReflectedType, Reflection_ReflectedType ) {
		StructInfo->Description = "A simple struct to test reflection";
		MAKE_DEFAULT();
		ADD_STATIC_CONSTANT( StaticImmutableShortValue, "Test Static Immutable Short Value" );
		ADD_MEMBER_CONSTANT( ImmutableByteValue, "Test Immutable Byte Value" );
		ADD_STATIC_VARIABLE( StaticShortValue, "Test Static Short Value" );
		ADD_MEMBER_VARIABLE( IntegerValue, "Test Integer Value" );
		ADD_MEMBER_VARIABLE( BooleanValue, "Test Boolean Value" );
	}
	STRUCT_TYPE_REFLECT_END( ReflectedType, "Reflection::ReflectedType" )
}
DEFINE_REFLECTION( ReflectedType, Reflection_ReflectedType );

namespace Reflection {
	STRUCT_TYPE_REFLECT_BEGIN( SecondReflectedType, Reflection_SecondReflectedType ) {
		StructInfo->Description = "Another struct to test reflection, in particular nesting";
		MAKE_DEFAULT();
		ADD_MEMBER_VARIABLE( VectorValue, "A vector that contains another reflected type" );
		ADD_MEMBER_VARIABLE( MapValue, "A map that contains strings" );
	}
	STRUCT_TYPE_REFLECT_END( SecondReflectedType, "Reflection::SecondReflectedType" )
}
DEFINE_REFLECTION( SecondReflectedType, Reflection_SecondReflectedType );

namespace Reflection {
	STRUCT_TYPE_REFLECT_BEGIN( RecursiveType, Reflection_RecursiveType ) {
		StructInfo->Description = "Recursive struct example";
		MAKE_DEFAULT();
		ADD_MEMBER_VARIABLE( Data, "Some data" );
		ADD_MEMBER_VARIABLE( Nodes, "Recursive list of the same type" );
	}
	STRUCT_TYPE_REFLECT_END( RecursiveType, "Reflection::RecursiveType" )
}
DEFINE_REFLECTION( RecursiveType, Reflection_RecursiveType );

namespace Reflection {
	STRUCT_TYPE_REFLECT_BEGIN( SerializedTypeA, Reflection_SerializedTypeA ) {
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
	STRUCT_TYPE_REFLECT_END( SerializedTypeA, "Reflection::SerializedTypeB" )
}
DEFINE_REFLECTION( SerializedTypeA, Reflection_SerializedTypeA );

namespace Reflection {
	STRUCT_TYPE_REFLECT_BEGIN( SerializedTypeB, Reflection_SerializedTypeB ) {
		StructInfo->Description = "Serialized type example B";
		MAKE_DEFAULT();
		MAKE_SERIALIZER();
		ADD_MEMBER_VARIABLE( CharValue, "" );
		ADD_MEMBER_VARIABLE( ShortValue, "" );
		ADD_MEMBER_VARIABLE( BooleanValue, "" );
		ADD_MEMBER_VARIABLE( DoubleValue, "" );
	}
	STRUCT_TYPE_REFLECT_END( SerializedTypeB, "Reflection::SerializedTypeB" )
}
DEFINE_REFLECTION( SerializedTypeB, Reflection_SerializedTypeB );
