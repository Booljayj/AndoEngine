#pragma once
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include "Reflection/Resolver/TypeResolver.h"

#define L_DECLARE_TYPE( __TYPE__ )\
extern TypeInfo const TypeInfo__##__TYPE__;\
template<> struct TypeResolver<std::decay<__TYPE__>::type> {\
	static TypeInfo const* Get() { return &TypeInfo__##__TYPE__; }\
}

namespace Reflection {
	//============================================================
	// Standard primitive types

	L_DECLARE_TYPE( void );

	L_DECLARE_TYPE( bool );
	L_DECLARE_TYPE( char );
	L_DECLARE_TYPE( int8_t );
	L_DECLARE_TYPE( uint8_t );
	L_DECLARE_TYPE( int16_t );
	L_DECLARE_TYPE( uint16_t );
	L_DECLARE_TYPE( int32_t );
	L_DECLARE_TYPE( uint32_t );
	L_DECLARE_TYPE( int64_t );
	L_DECLARE_TYPE( uint64_t );
	L_DECLARE_TYPE( size_t );

	L_DECLARE_TYPE( float );
	L_DECLARE_TYPE( double );
}

#undef L_DECLARE_TYPE
