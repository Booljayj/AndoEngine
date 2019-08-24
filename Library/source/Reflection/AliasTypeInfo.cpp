#include "Reflection/AliasTypeInfo.h"

namespace Reflection {
	AliasTypeInfo::AliasTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		VariableInfo const* InAliasedVariableInfo)
	: TypeInfo(
		AliasTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, AliasedVariableInfo(InAliasedVariableInfo)
	{}
}
