#include "Reflection/PolyTypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	PolyTypeInfo::PolyTypeInfo(
		Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
		TypeInfo const* inBaseTypeInfo, bool inCanBeBaseType, bool inCanBeDerivedType)
	: TypeInfo(
		PolyTypeInfo::Classification, inID, inDef,
		inDescription, inFlags, inSerializer)
	, baseType(inBaseTypeInfo)
	, canBeBaseType(inCanBeBaseType)
	, canBeDerivedType(inCanBeDerivedType)
	{}

	bool PolyTypeInfo::CanAssignType(PolyTypeInfo const* polyInfo, TypeInfo const* type) {
		if (!polyInfo || !type) return false;
		if (polyInfo->canBeBaseType && type == polyInfo->baseType) {
			return true;
		} else if (polyInfo->canBeDerivedType) {
			if (StructTypeInfo const* structType = type->As<StructTypeInfo>()) {
				return structType->DerivesFrom(polyInfo->baseType);
			}
		}
		return false;
	}
}
