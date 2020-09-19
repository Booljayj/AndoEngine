#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	TupleTypeInfo::TupleTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(TupleTypeInfo::Classification, inID, inDef)
	{}
}
