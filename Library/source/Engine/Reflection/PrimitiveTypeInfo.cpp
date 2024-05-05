#include "Engine/Reflection/PrimitiveTypeInfo.h"

/** Special definition for void, which is the only primitive type that cannot hold values. This exists for cases where void is a valid template parameter, and we want to display that */
namespace Reflection {
	struct EmptyPrimitiveTypeInfo : public PrimitiveTypeInfo {
		EmptyPrimitiveTypeInfo(Hash128 id, std::string_view name, std::string_view desc) : PrimitiveTypeInfo(PrimitiveTypeInfo::Classification, FTypeFlags::None(), id, name, MemoryParams{ 0, 0 }) {
			description = desc;
		}

		virtual void Destruct(void* instance) const final {}
		virtual void Construct(void* instance) const final {}
		virtual void Construct(void* instance, void const* other) const final {}
		virtual void Copy(void* instance, void const* other) const final {}
		virtual bool Equal(void const* instanceA, void const* instanceB) const final { return true; }

		virtual YAML::Node Serialize(void const* instance) const final { return YAML::Node{}; }
		virtual void Deserialize(YAML::Node const& node, void* instance) const final {}
	};

	EmptyPrimitiveTypeInfo const info_void{ "void"_h128, "void"sv, "Void type, used for missing type values"sv };
	EmptyPrimitiveTypeInfo const info_monostate{ "std::monostate"_h128, "std::monostate"sv, "Variant monostate type, used to indicate the variant can have a 'stateless' or 'default constructed' option"sv };

	PrimitiveTypeInfo const& Reflect<void>::Get() { return info_void; }
	PrimitiveTypeInfo const& Reflect<std::monostate>::Get() { return info_monostate; }

	#define L_DEFINE_REFLECT_PRIMITIVE(Type, TypeDescription)\
	TPrimitiveTypeInfo<Type> const info_##Type =\
		TPrimitiveTypeInfo<Type>{ #Type }\
		.Description(TypeDescription);\
	PrimitiveTypeInfo const& Reflect<Type>::Get() { return info_ ## Type; }

	#define L_DEFINE_REFLECT_NSPRIMITIVE(Namespace, Type, TypeDescription)\
	TPrimitiveTypeInfo<Namespace::Type> const info_ ## Type =\
		TPrimitiveTypeInfo<Namespace::Type>{ #Namespace "::" #Type }\
		.Description(TypeDescription);\
	PrimitiveTypeInfo const& Reflect<Namespace::Type>::Get() { return info_ ## Type; }

	L_DEFINE_REFLECT_NSPRIMITIVE(std, byte, "a byte of memory");
	L_DEFINE_REFLECT_PRIMITIVE(bool, "boolean");

	L_DEFINE_REFLECT_PRIMITIVE(int8_t, "signed 8-bit integer");
	L_DEFINE_REFLECT_PRIMITIVE(uint8_t, "unsigned 8-bit integer");
	L_DEFINE_REFLECT_PRIMITIVE(int16_t, "signed 16-bit integer");
	L_DEFINE_REFLECT_PRIMITIVE(uint16_t, "unsigned 16-bit integer");
	L_DEFINE_REFLECT_PRIMITIVE(int32_t, "signed 32-bit integer");
	L_DEFINE_REFLECT_PRIMITIVE(uint32_t, "unsigned 32-bit integer");
	L_DEFINE_REFLECT_PRIMITIVE(int64_t, "signed 64-bit integer");
	L_DEFINE_REFLECT_PRIMITIVE(uint64_t, "unsigned 64-bit integer");
	//L_DEFINE_REFLECT_PRIMITIVE(size_t, "size integer");

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

	#undef L_DEFINE_REFLECT_PRIMITIVE
	#undef L_DEFINE_REFLECT_NSPRIMITIVE
}
