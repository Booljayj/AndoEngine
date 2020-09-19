#include "Reflection/EnumerationTypeInfo.h"

namespace Reflection {
	EnumerationTypeInfo::EnumerationTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(EnumerationTypeInfo::Classification, inID, inDef)
	{}
}
