#include "Reflection/AliasTypeInfo.h"

namespace Reflection {
	AliasTypeInfo::AliasTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(AliasTypeInfo::Classification, inID, inDef)
	{}
}
