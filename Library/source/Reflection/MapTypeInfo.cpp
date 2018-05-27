#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo(
		std::string_view InName, size_t InSize, std::string_view InDescription,
		FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InKeyType, TypeInfo const* InValueType
	)
		: TypeInfo( CLASSIFICATION, InName, InSize, InDescription, InFlags, InSerializer )
		, KeyType( InKeyType )
		, ValueType( InValueType )
	{}
}