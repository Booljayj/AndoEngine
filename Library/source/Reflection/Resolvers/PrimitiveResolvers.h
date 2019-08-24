#pragma once
#include <string_view>
#include <string>
#include "Engine/Hash.h"
#include "Reflection/PrimitiveTypeInfo.h"

#define L_DECLARE_PRIMITIVE_TYPEINFO(_Type_)\
extern TPrimitiveTypeInfo<_Type_> const TypeInfo__##_Type_;\
template<> struct TypeResolver_Implementation<_Type_> {\
	static TypeInfo const* Get() { return &TypeInfo__##_Type_; }\
	static constexpr Hash128 GetID() { return Hash128{ #_Type_ }; }\
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
			static TPrimitiveTypeInfo<std::basic_string<CharType>> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() { return Hash128{ "std::basic_string" } + TypeResolver<CharType>::GetID(); }
		};
		template<typename CharType>
		TPrimitiveTypeInfo<std::basic_string<CharType>> const TypeResolver_Implementation<std::basic_string<CharType>>::_TypeInfo{ "dynamic string", FTypeFlags::None, nullptr };
	}
}

#undef L_DECLARE_PRIMITIVE_TYPEINFO
