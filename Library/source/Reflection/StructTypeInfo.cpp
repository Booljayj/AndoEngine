#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	StructTypeInfo::StructTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(StructTypeInfo::Classification, inID, inDef)
	{}
}
