#pragma once
#include <string_view>
#include <string>
#include "Engine/Hash.h"
#include "Reflection/PrimitiveTypeInfo.h"

#define L_DECLARE_PRIMITIVE_TYPEINFO( __TYPE__ )\
extern TPrimitiveTypeInfo<__TYPE__> const TypeInfo__##__TYPE__;\
template<> struct TypeResolver_Implementation<__TYPE__> {\
	static TypeInfo const* Get() { return &TypeInfo__##__TYPE__; }\
	static constexpr Hash128 GetID() { return Hash128{ #__TYPE__ }; }\
}

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard primitive type specializations

		//Special definition for void, which is the only primitive type that cannot hold values.
		extern TPrimitiveTypeInfo<void> const TypeInfo__void;
		template<> struct TypeResolver_Implementation<void> {
			static TypeInfo const* Get() { return &TypeInfo__void; }
			static constexpr Hash128 GetID() { return Hash128{}; }
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
		struct TypeResolver_Implementation<std::basic_string<TCHAR>> {
			static TPrimitiveTypeInfo<std::basic_string<TCHAR>> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() {
				return Hash128{ "std::basic_string" } + TypeResolver<TCHAR>::GetID();
			}
		};
		template<typename TCHAR>
		TPrimitiveTypeInfo<std::basic_string<TCHAR>> const TypeResolver_Implementation<std::basic_string<TCHAR>>::_TypeInfo{ "dynamic string", nullptr };
	}
}

#undef L_DECLARE_PRIMITIVE_TYPEINFO
