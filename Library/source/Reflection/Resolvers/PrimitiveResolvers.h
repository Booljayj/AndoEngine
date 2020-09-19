#pragma once
#include <string_view>
#include <string>
#include "Engine/Hash.h"
#include "Reflection/PrimitiveTypeInfo.h"

#define L_DECLARE_PRIMITIVE_TYPEINFO(Type)\
extern TPrimitiveTypeInfo<Type> const typeInfo_##Type;\
template<> struct TypeResolver_Implementation<Type> {\
	static TypeInfo const* Get() { return &typeInfo_##Type; }\
	static constexpr Hash128 GetID() { return Hash128{ #Type }; }\
}

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard primitive type specializations

		//Special definition for void, which is the only primitive type that cannot hold values.
		extern TPrimitiveTypeInfo<void> const typeInfo_void;
		template<> struct TypeResolver_Implementation<void> {
			static TypeInfo const* Get() { return &typeInfo_void; }
			static constexpr Hash128 GetID() { return Hash128{}; }
		};

		L_DECLARE_PRIMITIVE_TYPEINFO(bool);

		L_DECLARE_PRIMITIVE_TYPEINFO(int8_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(uint8_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(int16_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(uint16_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(int32_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(uint32_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(int64_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(uint64_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(size_t);

		L_DECLARE_PRIMITIVE_TYPEINFO(char);
		//L_DECLARE_PRIMITIVE_TYPEINFO(char8_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(char16_t);
		L_DECLARE_PRIMITIVE_TYPEINFO(char32_t);

		L_DECLARE_PRIMITIVE_TYPEINFO(float);
		L_DECLARE_PRIMITIVE_TYPEINFO(double);
		//using ldouble_t = long double; L_DECLARE_PRIMITIVE_TYPEINFO(ldouble);

		//============================================================
		// String primitive types

		template<typename CharType>
		struct TypeResolver_Implementation<std::basic_string<CharType>> {
			static TPrimitiveTypeInfo<std::basic_string<CharType>> const typeInfo;
			static TypeInfo const* Get() { return &typeInfo; }
			static constexpr Hash128 GetID() { return Hash128{ "std::basic_string" } + TypeResolver<CharType>::GetID(); }
		};
		template<typename CharType>
		TPrimitiveTypeInfo<std::basic_string<CharType>> const TypeResolver_Implementation<std::basic_string<CharType>>::typeInfo = TPrimitiveTypeInfo<std::basic_string<CharType>>{}
			.Description("dynamic string");
	}
}

#undef L_DECLARE_PRIMITIVE_TYPEINFO
