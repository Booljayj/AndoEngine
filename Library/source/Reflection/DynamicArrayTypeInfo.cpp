#include "Reflection/DynamicArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	DynamicArrayTypeInfo::DynamicArrayTypeInfo( std::string_view InName, size_t InSize, std::string_view InDescription, TypeInfo const* InElementType )
	: TypeInfo( CLASSIFICATION, InName, InSize, InDescription, FTypeFlags::None, nullptr )
	, ElementType( InElementType )
	{}
}
