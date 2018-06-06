#include "Reflection/FixedArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	FixedArrayTypeInfo::FixedArrayTypeInfo( std::string_view InName, size_t InSize, std::string_view InDescription, TypeInfo const* InElementType, size_t InCount )
	: TypeInfo( CLASSIFICATION, InName, InSize, InDescription, FTypeFlags::None, new Serialization::FixedArraySerializer( this ) )
	, ElementType( InElementType )
	, Count( InCount )
	{}
}
