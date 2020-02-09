#include "Reflection/AliasTypeInfo.h"

namespace Reflection {
	AliasTypeInfo::AliasTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		VariableInfo const* inVariableInfo)
	: TypeInfo(
		AliasTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, variableInfo(inVariableInfo)
	{}
}
