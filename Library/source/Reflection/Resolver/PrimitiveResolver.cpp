#include "Reflection/Resolver/PrimitiveResolver.h"
#include "Engine/StringID.h"
#include "Reflection/TypeInfo.h"

#define NAME( __TYPE__ ) TypeInfo__##__TYPE__

#define TYPEINFO( __TYPE__ ) TypeInfo NAME( __TYPE__ ) { nullptr, #__TYPE__, sizeof( __TYPE__ ) }

namespace Reflection
{
	TypeInfo NAME( void ) { nullptr, "void", 0 }; //cannot use sizeof( void )

	TYPEINFO( bool );
	TYPEINFO( char );

	TYPEINFO( int8_t );
	TYPEINFO( uint8_t );
	TYPEINFO( int16_t );
	TYPEINFO( uint16_t );
	TYPEINFO( int32_t );
	TYPEINFO( uint32_t );
	TYPEINFO( int64_t );
	TYPEINFO( uint64_t );
	TYPEINFO( size_t );

	TYPEINFO( float );
	TYPEINFO( double );
}
