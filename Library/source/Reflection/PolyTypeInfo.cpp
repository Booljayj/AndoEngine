#include "Reflection/PolyTypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	PolyTypeInfo::PolyTypeInfo(Hash128 inID, CompilerDefinition inDef)
	: TypeInfo(PolyTypeInfo::Classification, inID, inDef)
	, canBeBaseType(false)
	, canBeDerivedType(false)
	{}

	bool PolyTypeInfo::CanAssignType(TypeInfo const* type) const {
		if (!type) return false;

		if (canBeBaseType && type == baseType) {
			return true;
		} else if (canBeDerivedType) {
			if (StructTypeInfo const* structType = type->As<StructTypeInfo>()) {
				return structType->DerivesFrom(baseType);
			}
		}
		return false;
	}
}
