#include "Reflection/ArrayTypeInfo.h"

namespace Reflection {
	ArrayTypeInfo::ArrayTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		bool InIsFixedSize, TypeInfo const* InElementTypeInfo)
	: TypeInfo(
		ArrayTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, IsFixedSize(InIsFixedSize)
	, ElementTypeInfo(InElementTypeInfo)
	{}
}
