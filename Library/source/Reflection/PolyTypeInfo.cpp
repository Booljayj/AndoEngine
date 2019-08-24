#include "Reflection/PolyTypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	PolyTypeInfo::PolyTypeInfo(
		Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		TypeInfo const* InBaseTypeInfo, bool InCanBeBaseType, bool InCanBeDerivedType)
	: TypeInfo(
		PolyTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer)
	, BaseTypeInfo(InBaseTypeInfo)
	, CanBeBaseType(InCanBeBaseType)
	, CanBeDerivedType(InCanBeDerivedType)
	{}

	bool PolyTypeInfo::CanAssignType(PolyTypeInfo const* PolyInfo, TypeInfo const* Info) {
		if (!PolyInfo || !Info) return false;
		if (PolyInfo->CanBeBaseType && Info == PolyInfo->BaseTypeInfo) {
			return true;
		} else if (PolyInfo->CanBeDerivedType) {
			if (StructTypeInfo const* StructInfo = Info->As<StructTypeInfo>()) {
				return StructInfo->DerivesFrom(PolyInfo->BaseTypeInfo);
			}
		}
		return false;
	}
}
