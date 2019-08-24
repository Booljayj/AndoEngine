#include "Reflection/EnumerationTypeInfo.h"

namespace Reflection {
	EnumerationTypeInfo::EnumerationTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InUnderlyingTypeInfo)
	: TypeInfo(
		EnumerationTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, UnderlyingTypeInfo(InUnderlyingTypeInfo)
	{}
}
