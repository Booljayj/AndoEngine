#include "Reflection/EnumerationTypeInfo.h"

namespace Reflection {
	EnumerationTypeInfo::EnumerationTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		TypeInfo const* inUnderlyingType)
	: TypeInfo(
		EnumerationTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, underlyingType(inUnderlyingType)
	{}
}
