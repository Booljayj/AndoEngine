#pragma once
#include <string_view>
#include <string>
#include "Reflection/TypeInfo.h"
#include "Reflection/StringTypeInfo.h"

#define L_DECLARE_PRIMITIVE_TYPEINFO( __TYPE__ )\
extern TypeInfo const TypeInfo__##__TYPE__;\
template<> struct TypeResolver<__TYPE__> {\
	static TypeInfo const* Get() { return &TypeInfo__##__TYPE__; }\
	static std::string_view GetName() { return #__TYPE__; }\
}

namespace Reflection
{
	/** Global reflection accessor type, specialized for types which are known to the reflection system */
	template<typename TTYPE>
	struct TypeResolver {
		static TypeInfo const* Get() { return nullptr; }
		static std::string_view GetName() { return "{{UNKNOWN}}"; }
	};

	//============================================================
	// Standard primitive type specializations

	L_DECLARE_PRIMITIVE_TYPEINFO( void );

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

	extern StringTypeInfo const TypeInfo__std_string;
	template<> struct TypeResolver<std::string> {
		static TypeInfo const* Get() { return &TypeInfo__std_string; }
		static std::string_view GetName() { return "std::string"; }
	};

	//============================================================
	// Numeric template argument partial specialization

	template<typename T, size_t SIZE>
	struct TypeResolver<std::integral_constant<T, SIZE>> {
		static std::string_view GetName() {
			static std::string const Name{ std::to_string( SIZE ) };
			return Name;
		}
	};
}

#undef L_DECLARE_PRIMITIVE_TYPEINFO
