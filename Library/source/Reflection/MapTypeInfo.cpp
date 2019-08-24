#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InKeyTypeInfo, TypeInfo const* InValueTypeInfo)
	: TypeInfo(
		MapTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, KeyTypeInfo(InKeyTypeInfo)
	, ValueTypeInfo(InValueTypeInfo)
	{}
}
