#include "Reflection/SetTypeInfo.h"

namespace Reflection {
	SetTypeInfo::SetTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InValueTypeInfo)
	: TypeInfo(
		SetTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, ValueTypeInfo(InValueTypeInfo)
	{}
}
