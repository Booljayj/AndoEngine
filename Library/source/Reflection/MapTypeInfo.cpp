#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo( std::string_view InName, size_t InSize, std::string_view InDescription, TypeInfo const* InKeyType, TypeInfo const* InValueType )
	: TypeInfo( CLASSIFICATION, InName, InSize, InDescription, FTypeFlags::None, nullptr )
	, KeyType( InKeyType )
	, ValueType( InValueType )
	{}
}