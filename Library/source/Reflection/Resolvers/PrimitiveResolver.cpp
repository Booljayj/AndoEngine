#include "Serialization/PrimitiveSerializer.h"
#include "Reflection/Resolvers/PrimitiveResolvers.h"

#define DEFINE_PRIMITIVE_TYPEINFO(Type, DescriptionString)\
TPrimitiveTypeInfo<Type> const typeInfo_##Type = TPrimitiveTypeInfo<Type>{}.Description(DescriptionString)

namespace Reflection {
	namespace Internal {
		TPrimitiveTypeInfo<void> const typeInfo_void{};

		DEFINE_PRIMITIVE_TYPEINFO(bool, "boolean value");

		DEFINE_PRIMITIVE_TYPEINFO(int8_t, "signed 8-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(uint8_t, "unsigned 8-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(int16_t, "signed 16-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(uint16_t, "unsigned 16-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(int32_t, "signed 32-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(uint32_t, "unsigned 32-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(int64_t, "signed 64-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(uint64_t, "unsigned 64-bit integer");
		DEFINE_PRIMITIVE_TYPEINFO(size_t, "size integer");

		DEFINE_PRIMITIVE_TYPEINFO(char, "single-byte value");
		//DEFINE_PRIMITIVE_TYPEINFO(char8_t, "8-bit character value);
		DEFINE_PRIMITIVE_TYPEINFO(char16_t, "16-bit character value");
		DEFINE_PRIMITIVE_TYPEINFO(char32_t, "32-bit character value");

		DEFINE_PRIMITIVE_TYPEINFO(float, "single-precision number");
		DEFINE_PRIMITIVE_TYPEINFO(double, "double-precision number");
		//DEFINE_PRIMITIVE_TYPEINFO(ldouble_t, "extreme-precision number");
	}
}
