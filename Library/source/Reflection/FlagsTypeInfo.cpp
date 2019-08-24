#include "Reflection/FlagsTypeInfo.h"

namespace Reflection {
	FlagsTypeInfo::FlagsTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InUnderlyingTypeInfo)
	: TypeInfo(
		FlagsTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, UnderlyingTypeInfo(InUnderlyingTypeInfo)
	{}
}
