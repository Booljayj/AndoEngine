#include "Reflection/DynamicArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	DynamicArrayTypeInfo::DynamicArrayTypeInfo(
		std::string_view InName, size_t InSize, std::string_view InDescription,
		FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InElementType
	)
		: TypeInfo( CLASSIFICATION, InName, InSize, InDescription, InFlags, InSerializer )
		, ElementType( InElementType )
	{}
}
