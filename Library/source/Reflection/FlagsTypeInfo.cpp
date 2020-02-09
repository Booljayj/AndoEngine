#include "Reflection/FlagsTypeInfo.h"

namespace Reflection {
	FlagsTypeInfo::FlagsTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		TypeInfo const* inUnderlyingType)
	: TypeInfo(
		FlagsTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, underlyingType(inUnderlyingType)
	{}
}
