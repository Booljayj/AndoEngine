#include "Reflection/FlagsTypeInfo.h"

namespace Reflection {
	FlagsTypeInfo::FlagsTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(FlagsTypeInfo::Classification, inID, inDef)
	{}
}
