#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo(
		sid_t InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, Serialization::ISerializer* InSerializer,
		TypeInfo const* InKeyType, TypeInfo const* InValueType
	)
	: TypeInfo(
		MapTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, FTypeFlags::None, InSerializer )
	, KeyType( InKeyType )
	, ValueType( InValueType )
	{}
}