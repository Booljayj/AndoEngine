#include "Reflection/ArrayTypeInfo.h"

namespace Reflection {
	ArrayTypeInfo::ArrayTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		bool inIsFixedSize, TypeInfo const* inElementType)
	: TypeInfo(
		ArrayTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, isFixedSize(inIsFixedSize)
	, elementType(inElementType)
	{}
}
