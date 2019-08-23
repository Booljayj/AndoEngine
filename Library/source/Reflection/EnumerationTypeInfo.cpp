#include "Reflection/EnumerationTypeInfo.h"

namespace Reflection {
	EnumerationTypeInfo::EnumerationTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InUnderlying
	)
	: TypeInfo(
		EnumerationTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, Underlying(InUnderlying)
	{}
}
