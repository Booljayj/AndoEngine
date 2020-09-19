#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(MapTypeInfo::Classification, inID, inDef)
	{}
}
