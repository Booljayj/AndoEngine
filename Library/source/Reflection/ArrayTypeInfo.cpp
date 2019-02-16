#include "Reflection/ArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	ArrayTypeInfo::ArrayTypeInfo(
		sid_t InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, Serialization::ISerializer* InSerializer,
		bool InIsFixedSize, TypeInfo const* InElementType
	)
	: TypeInfo(
		ArrayTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, FTypeFlags::None, InSerializer )
	, IsFixedSize( InIsFixedSize )
	, ElementType( InElementType )
	{}
}
