#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	StructTypeInfo::StructTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		StructTypeInfo const* inBaseType, void const* inDefaults,
		Fields inStatics, Fields inMembers)
	: TypeInfo(
		StructTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, baseType(inBaseType)
	, defaults(inDefaults)
	, statics(inStatics)
	, members(inMembers)
	{}
}
