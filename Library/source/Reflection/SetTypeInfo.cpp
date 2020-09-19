#include "Reflection/SetTypeInfo.h"

namespace Reflection {
	SetTypeInfo::SetTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(SetTypeInfo::Classification, inID, inDef)
	{}
}
