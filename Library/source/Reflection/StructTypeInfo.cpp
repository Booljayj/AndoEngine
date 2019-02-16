#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	StructTypeInfo::StructTypeInfo(
		sid_t InUniqueID, CompilerDefinition InDefinition,
		const char* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
		StructTypeInfo const* InBaseType, void const* InDefault,
		Fields InStatic, Fields InMember
	)
	: TypeInfo(
		StructTypeInfo::CLASSIFICATION, InUniqueID, InDefinition,
		InDescription, InFlags, InSerializer )
	, BaseType( InBaseType )
	, Default( InDefault )
	, Static( InStatic )
	, Member( InMember )
	{}

	void StructTypeInfo::GetStaticConstantsRecursive( std::vector<ConstantInfo const*>& OutStaticConstants ) const {
		OutStaticConstants.reserve( OutStaticConstants.size() + Static.Constants.size() );
		for( auto const& StaticConstant : Static.Constants ) {
			OutStaticConstants.push_back( StaticConstant );
		}
		if( BaseType ) BaseType->GetStaticConstantsRecursive( OutStaticConstants );
	}

	void StructTypeInfo::GetMemberConstantsRecursive( std::vector<ConstantInfo const*>& OutMemberConstants ) const {
		OutMemberConstants.reserve( OutMemberConstants.size() + Member.Constants.size() );
		for( auto const& MemberConstant : Member.Constants ) {
			OutMemberConstants.push_back( MemberConstant );
		}
		if( BaseType ) BaseType->GetMemberConstantsRecursive( OutMemberConstants );
	}

	void StructTypeInfo::GetStaticVariablesRecursive( std::vector<VariableInfo const*>& OutStaticVariables ) const {
		OutStaticVariables.reserve( OutStaticVariables.size() + Static.Variables.size() );
		for( auto const& StaticVariable : Static.Variables ) {
			OutStaticVariables.push_back( StaticVariable );
		}
		if( BaseType ) BaseType->GetStaticVariablesRecursive( OutStaticVariables );
	}

	void StructTypeInfo::GetMemberVariablesRecursive( std::vector<VariableInfo const*>& OutMemberVariables ) const {
		OutMemberVariables.reserve( OutMemberVariables.size() + Member.Variables.size() );
		for( auto const& MemberVariable : Member.Variables ) {
			OutMemberVariables.push_back( MemberVariable );
		}
		if( BaseType ) BaseType->GetMemberVariablesRecursive( OutMemberVariables );
	}
}
