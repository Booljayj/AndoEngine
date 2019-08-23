#include "Reflection/AliasTypeInfo.h"

namespace Reflection {
	AliasTypeInfo::AliasTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		VariableInfo const* InVariable
	)
	: TypeInfo(
		AliasTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, Variable(InVariable)
	{}
}
