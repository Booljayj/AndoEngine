#pragma once
#include <string_view>
#include <string>
#include "Reflection/PrimitiveTypeInfo.h"

#define L_DECLARE_PRIMITIVE_TYPEINFO( __TYPE__ )\
extern TPrimitiveTypeInfo<__TYPE__> const TypeInfo__##__TYPE__;\
template<> struct TypeResolver<__TYPE__> {\
	static TypeInfo const* Get() { return &TypeInfo__##__TYPE__; }\
	static constexpr sid_t GetID() { return id( #__TYPE__ ); }\
}

namespace Reflection {
	//============================================================
	// Standard primitive type specializations

	//Special definition for void, which is the only primitive type that cannot hold values.
	extern TPrimitiveTypeInfo<void> const TypeInfo__void;
	template<> struct TypeResolver<void> {
		static TypeInfo const* Get() { return &TypeInfo__void; }
		static constexpr sid_t GetID() { return 0; }
	};

	L_DECLARE_PRIMITIVE_TYPEINFO( bool );
	L_DECLARE_PRIMITIVE_TYPEINFO( char );

	L_DECLARE_PRIMITIVE_TYPEINFO( int8_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( uint8_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( int16_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( uint16_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( int32_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( uint32_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( int64_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( uint64_t );
	L_DECLARE_PRIMITIVE_TYPEINFO( size_t );

	L_DECLARE_PRIMITIVE_TYPEINFO( float );
	L_DECLARE_PRIMITIVE_TYPEINFO( double );

	//============================================================
	// String primitive types

	template<typename TCHAR>
	struct TypeResolver<std::basic_string<TCHAR>> {
		static TPrimitiveTypeInfo<std::basic_string<TCHAR>> const InstancedTypeInfo{ "dynamic string", nullptr };
		static TypeInfo const* Get() { return &InstancedTypeInfo; }
		static constexpr sid_t GetID() { return id_combine( id( "std::basic_string" ), TypeResolver<TCHAR>::GetID() ); }
	};
}

#undef L_DECLARE_PRIMITIVE_TYPEINFO
