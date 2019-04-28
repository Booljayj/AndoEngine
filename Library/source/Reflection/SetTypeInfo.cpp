#include "Reflection/SetTypeInfo.h"

namespace Reflection {
	SetTypeInfo::SetTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, Serialization::ISerializer* InSerializer,
		TypeInfo const* InValueType
	)
	: TypeInfo(
		SetTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, FTypeFlags::None, InSerializer )
	, ValueType( InValueType )
	{}
}
