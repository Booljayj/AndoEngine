#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	TupleTypeInfo::TupleTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		size_t InSize)
	: TypeInfo(
		TupleTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, Size(InSize)
	{}
}
