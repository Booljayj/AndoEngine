#pragma once
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	/** TypeInfo for a primitive type */
	struct PrimitiveTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Primitive;
		virtual ~PrimitiveTypeInfo() = default;
	};

	//============================================================
	// Templates

	template<typename Type>
	struct TPrimitiveTypeInfo : public ImplementedTypeInfo<Type, PrimitiveTypeInfo> {
		TPrimitiveTypeInfo(std::string_view name) : ImplementedTypeInfo<Type, PrimitiveTypeInfo>(Reflect<Type>::ID, name) {}
	};

	//============================================================
	// Standard primitive reflection

	#define L_REFLECT_PRIMITIVE(Type)\
	template<> struct Reflect<Type> {\
		static PrimitiveTypeInfo const& Get();\
		static constexpr Hash128 ID = Hash128{ #Type };\
	}

	L_REFLECT_PRIMITIVE(void);
	L_REFLECT_PRIMITIVE(std::byte);
	L_REFLECT_PRIMITIVE(bool);

	L_REFLECT_PRIMITIVE(int8_t);
	L_REFLECT_PRIMITIVE(uint8_t);
	L_REFLECT_PRIMITIVE(int16_t);
	L_REFLECT_PRIMITIVE(uint16_t);
	L_REFLECT_PRIMITIVE(int32_t);
	L_REFLECT_PRIMITIVE(uint32_t);
	L_REFLECT_PRIMITIVE(int64_t);
	L_REFLECT_PRIMITIVE(uint64_t);
	//L_REFLECT_PRIMITIVE(size_t);

	L_REFLECT_PRIMITIVE(char);
	L_REFLECT_PRIMITIVE(char8_t);
	L_REFLECT_PRIMITIVE(char16_t);
	L_REFLECT_PRIMITIVE(char32_t);

	L_REFLECT_PRIMITIVE(float);
	L_REFLECT_PRIMITIVE(double);
	//L_REFLECT_PRIMITIVE(ldouble_t);

	//@todo For now, we consider strings to be primitives. This may change in the future to support more elaborate string operations.
	L_REFLECT_PRIMITIVE(std::string);
	L_REFLECT_PRIMITIVE(std::u8string);
	L_REFLECT_PRIMITIVE(std::u16string);
	L_REFLECT_PRIMITIVE(std::u32string);

	/** The variant monostate is considered to be a primitive, even though it's a struct */
	L_REFLECT_PRIMITIVE(std::monostate);

	#undef L_REFLECT_PRIMITIVE
}
