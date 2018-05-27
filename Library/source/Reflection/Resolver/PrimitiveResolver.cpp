#include <memory>
#include "Reflection/Resolver/PrimitiveResolver.h"
#include "Engine/StringID.h"
#include "Reflection/TypeInfo.h"
#include "Serialization/PrimitiveSerializer.h"

#define NAME( __TYPE__ ) TypeInfo__##__TYPE__

#define TYPEINFO( __TYPE__, __SIZE__, __DESCRIPTION__ )\
TypeInfo const TypeInfo__##__TYPE__ {\
	TypeInfo::CLASSIFICATION,\
	TypeResolver<__TYPE__>::GetName(),\
	sizeof( __TYPE__ ),\
	__DESCRIPTION__,\
	(FTypeFlags)0,\
	new Serialization::TPrimitiveSerializer<__TYPE__>()\
}

namespace Reflection
{
	TypeInfo const TypeInfo__void{
		TypeInfo::CLASSIFICATION,
		TypeResolver<void>::GetName(), 0, "not a type",
		FTypeFlags::None, nullptr
	};

	TYPEINFO( bool, sizeof( bool ), "boolean value" );
	TYPEINFO( char, sizeof( char ), "single-byte value" );

	TYPEINFO( int8_t, sizeof( int8_t ), "signed 8-bit integer" );
	TYPEINFO( uint8_t, sizeof( uint8_t ), "unsigned 8-bit integer" );
	TYPEINFO( int16_t, sizeof( int16_t ), "signed 16-bit integer" );
	TYPEINFO( uint16_t, sizeof( uint16_t ), "unsigned 16-bit integer" );
	TYPEINFO( int32_t, sizeof( int32_t ), "signed 32-bit integer" );
	TYPEINFO( uint32_t, sizeof( uint32_t ), "unsigned 32-bit integer" );
	TYPEINFO( int64_t, sizeof( int64_t ), "signed 64-bit integer" );
	TYPEINFO( uint64_t, sizeof( uint64_t ), "unsigned 64-bit integer" );
	TYPEINFO( size_t, sizeof( size_t ), "size integer" );

	TYPEINFO( float, sizeof( float ), "single-precision number" );
	TYPEINFO( double, sizeof( double ), "double-precision number" );
}
