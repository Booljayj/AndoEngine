#include "Reflection/ArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	ArrayTypeInfo::ArrayTypeInfo(
		sid_t InUniqueID, size_t InSize, size_t InAlignment,
		char const* InMangledName, char const* InDescription,
		Serialization::ISerializer* InSerializer,
		bool InIsFixedSize, TypeInfo const* InElementType
	)
	: TypeInfo(
		ArrayTypeInfo::CLASSIFICATION,
		InUniqueID, InSize, InAlignment,
		InMangledName, InDescription,
		FTypeFlags::None, InSerializer )
	, IsFixedSize( InIsFixedSize )
	, ElementType( InElementType )
	{}
}
