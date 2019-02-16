#include <memory>
#include "Reflection/Resolver/PrimitiveResolver.h"
#include "Reflection/TypeInfo.h"
#include "Serialization/PrimitiveSerializer.h"

#define DEFINE_PRIMITIVE_TYPEINFO( __TYPE__, __DESCRIPTION__ )\
TPrimitiveTypeInfo<__TYPE__> const TypeInfo__##__TYPE__ { __DESCRIPTION__, nullptr }

namespace Reflection
{
	TPrimitiveTypeInfo<void> const TypeInfo__void{};

	DEFINE_PRIMITIVE_TYPEINFO( bool, "boolean value" );
	DEFINE_PRIMITIVE_TYPEINFO( char, "single-byte value" );

	DEFINE_PRIMITIVE_TYPEINFO( int8_t, "signed 8-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( uint8_t, "unsigned 8-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( int16_t, "signed 16-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( uint16_t, "unsigned 16-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( int32_t, "signed 32-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( uint32_t, "unsigned 32-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( int64_t, "signed 64-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( uint64_t, "unsigned 64-bit integer" );
	DEFINE_PRIMITIVE_TYPEINFO( size_t, "size integer" );

	DEFINE_PRIMITIVE_TYPEINFO( float, "single-precision number" );
	DEFINE_PRIMITIVE_TYPEINFO( double, "double-precision number" );
}
