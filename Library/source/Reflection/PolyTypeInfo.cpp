#include "Reflection/PolyTypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	PolyTypeInfo::PolyTypeInfo(
		sid_t InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, Serialization::ISerializer* InSerializer,
		TypeInfo const* InBaseType, bool InCanBeBaseType, bool InCanBeDerivedType
	)
	: TypeInfo(
		PolyTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, FTypeFlags::None, InSerializer )
	, BaseType( InBaseType ), CanBeBaseType( InCanBeBaseType ), CanBeDerivedType( InCanBeDerivedType )
	{}

	bool PolyTypeInfo::CanAssignType( PolyTypeInfo const* PolyInfo, TypeInfo const* Info ) {
		if( !PolyInfo || !Info ) return false;
		if( PolyInfo->CanBeBaseType && Info == PolyInfo->BaseType ){
			return true;
		} else if( PolyInfo->CanBeDerivedType ) {
			if( StructTypeInfo const* StructInfo = Info->As<StructTypeInfo>() ) {
				return StructInfo->DerivesFrom( PolyInfo->BaseType );
			}
		}
		return false;
	}
}
