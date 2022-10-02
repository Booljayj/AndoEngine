#include "Engine/Reflection/PrimitiveTypeInfo.h"

/** Special definition for void, which is the only primitive type that cannot hold values. This exists for cases where void is a valid template parameter, and we want to display that */
namespace Reflection {
	struct VoidTypeInfo : public PrimitiveTypeInfo {
		VoidTypeInfo() : PrimitiveTypeInfo(PrimitiveTypeInfo::Classification, GetLibrary(), Hash128{ "void"sv }, "void"sv, FTypeFlags::None(), MemoryParams{ 0, 0 }) {
			description = "void type"sv;
		}

		virtual void Destruct(void* instance) const final {}
		virtual void Construct(void* instance) const final {}
		virtual void Construct(void* instance, void const* other) const final {}
		virtual void Copy(void* instance, void const* other) const final {}
		virtual bool Equal(void const* instanceA, void const* instanceB) const final { return false; }
	};
	VoidTypeInfo const info_void;
}
::Reflection::TypeInfo const* Reflect<void>::Get() { return &::Reflection::info_void; }

#define L_DEFINE_REFLECT_PRIMITIVE(Type, TypeDescription)\
namespace Reflection {\
	TPrimitiveTypeInfo<Type> const info_##Type = TPrimitiveTypeInfo<Type>{ #Type }\
		.Description(TypeDescription);\
}\
::Reflection::TypeInfo const* Reflect<Type>::Get() { return &::Reflection::info_ ## Type; }

#define L_DEFINE_REFLECT_NSPRIMITIVE(Namespace, Type, TypeDescription)\
namespace Reflection {\
	TPrimitiveTypeInfo<Namespace::Type> const info_ ## Type = TPrimitiveTypeInfo<Namespace::Type>{ #Namespace "::" #Type }\
		.Description(TypeDescription);\
}\
::Reflection::TypeInfo const* Reflect<Namespace::Type>::Get() { return &::Reflection::info_ ## Type; }

L_DEFINE_REFLECT_PRIMITIVE(bool, "boolean");

L_DEFINE_REFLECT_PRIMITIVE(int8_t, "signed 8-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(uint8_t, "unsigned 8-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(int16_t, "signed 16-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(uint16_t, "unsigned 16-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(int32_t, "signed 32-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(uint32_t, "unsigned 32-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(int64_t, "signed 64-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(uint64_t, "unsigned 64-bit integer");
L_DEFINE_REFLECT_PRIMITIVE(size_t, "size integer");

L_DEFINE_REFLECT_PRIMITIVE(char, "single-byte");
L_DEFINE_REFLECT_PRIMITIVE(char8_t, "8-bit character");
L_DEFINE_REFLECT_PRIMITIVE(char16_t, "16-bit character");
L_DEFINE_REFLECT_PRIMITIVE(char32_t, "32-bit character");

L_DEFINE_REFLECT_PRIMITIVE(float, "single-precision number");
L_DEFINE_REFLECT_PRIMITIVE(double, "double-precision number");
//L_DEFINE_REFLECT_PRIMITIVE(ldouble_t, "extreme-precision number");

L_DEFINE_REFLECT_NSPRIMITIVE(std, string, "ansi string");
L_DEFINE_REFLECT_NSPRIMITIVE(std, u8string, "UTF-8 string");
L_DEFINE_REFLECT_NSPRIMITIVE(std, u16string, "UTF-16 string");
L_DEFINE_REFLECT_NSPRIMITIVE(std, u32string, "UTF-32 string");

L_DEFINE_REFLECT_NSPRIMITIVE(std, monostate, "variant monostate");

#undef L_DEFINE_REFLECT_PRIMITIVE
#undef L_DEFINE_REFLECT_NSPRIMITIVE
