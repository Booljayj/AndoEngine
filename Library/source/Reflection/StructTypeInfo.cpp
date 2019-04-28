#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	StructTypeInfo::StructTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		StructTypeInfo const* InBaseType, void const* InDefault,
		Fields InStatic, Fields InMember
	)
	: TypeInfo(
		StructTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer )
	, BaseType( InBaseType ), Default( InDefault )
	, Static( InStatic ), Member( InMember )
	{}
}
