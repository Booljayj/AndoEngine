#include "Reflection/DynamicArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	ArrayTypeInfo::ArrayTypeInfo( std::string_view InName, size_t InSize, std::string_view InDescription, bool InIsFixedSize, TypeInfo const* InElementType )
	: TypeInfo( CLASSIFICATION, InName, InSize, InDescription, FTypeFlags::None, nullptr )
	, IsFixedSize( InIsFixedSize )
	, ElementType( InElementType )
	{}
}
