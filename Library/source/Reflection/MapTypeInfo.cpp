#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		TypeInfo const* inKeyType, TypeInfo const* inValueType)
	: TypeInfo(
		MapTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, keyType(inKeyType)
	, valueType(inValueType)
	{}
}
