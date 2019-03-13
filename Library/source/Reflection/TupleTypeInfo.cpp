#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	TupleTypeInfo::TupleTypeInfo(
		sid_t InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, Serialization::ISerializer* InSerializer,
		size_t InSize
	)
	: TypeInfo(
		TupleTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, FTypeFlags::None, InSerializer )
	, Size( InSize )
	{}
}
