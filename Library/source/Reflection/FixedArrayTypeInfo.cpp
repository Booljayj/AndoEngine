#include "Reflection/FixedArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	FixedArrayTypeInfo::FixedArrayTypeInfo(
		std::string_view InName, size_t InSize, std::string_view InDescription,
		FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InElementType, size_t InCount
	)
		: TypeInfo( CLASSIFICATION, InName, InSize, InDescription, InFlags, InSerializer )
		, ElementType( InElementType )
		, Count( InCount )
	{}
}
