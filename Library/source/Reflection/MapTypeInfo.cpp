#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo(
		sid_t InUniqueID, size_t InSize, size_t InAlignment,
		char const* InMangledName, char const* InDescription,
		Serialization::ISerializer* InSerializer,
		TypeInfo const* InKeyType, TypeInfo const* InValueType
	)
	: TypeInfo(
		MapTypeInfo::CLASSIFICATION,
		InUniqueID, InSize, InAlignment,
		InMangledName, InDescription,
		FTypeFlags::None, InSerializer )
	, KeyType( InKeyType )
	, ValueType( InValueType )
	{}
}