#include "Reflection/SetTypeInfo.h"

namespace Reflection {
	SetTypeInfo::SetTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		TypeInfo const* inValueType)
	: TypeInfo(
		SetTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, valueType(inValueType)
	{}
}
