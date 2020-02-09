#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	TupleTypeInfo::TupleTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		size_t inSize)
	: TypeInfo(
		TupleTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, size(inSize)
	{}
}
