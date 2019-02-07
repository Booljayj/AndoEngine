#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo( std::string_view InName, size_t InSize, size_t InAlignment, std::string_view InDescription, TypeInfo const* InKeyType, TypeInfo const* InValueType )
	: TypeInfo( CLASSIFICATION, InName, InSize, InAlignment, InDescription, FTypeFlags::None, nullptr )
	, KeyType( InKeyType )
	, ValueType( InValueType )
	{}
}