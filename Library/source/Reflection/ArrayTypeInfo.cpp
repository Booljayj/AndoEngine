#include "Reflection/ArrayTypeInfo.h"

namespace Reflection {
	ArrayTypeInfo::ArrayTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(ArrayTypeInfo::Classification, inID, inDef)
	{}
}
