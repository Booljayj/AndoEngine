#include "Reflection/FlagsTypeInfo.h"

namespace Reflection {
	FlagsTypeInfo::FlagsTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InUnderlying
	)
	: TypeInfo(
		FlagsTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, Underlying(InUnderlying)
	{}
}
