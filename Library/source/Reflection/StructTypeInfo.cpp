#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	StructTypeInfo::StructTypeInfo( std::string_view InName, size_t InSize, void (*Initializer)( StructTypeInfo* ) )
	: TypeInfo( StructTypeInfo::CLASSIFICATION, InName, InSize )
	{
		if( Initializer ) Initializer( this );
	}

	void StructTypeInfo::GetStaticConstantsRecursive( std::vector<StaticConstantInfo const*>& OutStaticConstants ) const {
		if( BaseType ) {
			BaseType->GetStaticConstantsRecursive( OutStaticConstants );
		}
		for( auto const& StaticConstant : StaticConstants ) {
			OutStaticConstants.push_back( StaticConstant.get() );
		}
	}

	void StructTypeInfo::GetStaticVariablesRecursive( std::vector<StaticVariableInfo const*>& OutStaticVariables ) const {
		if( BaseType ) {
			BaseType->GetStaticVariablesRecursive( OutStaticVariables );
		}
		for( auto const& StaticVariable : StaticVariables ) {
			OutStaticVariables.push_back( StaticVariable.get() );
		}
	}

	void StructTypeInfo::GetMemberConstantsRecursive( std::vector<MemberConstantInfo const*>& OutMemberConstants ) const {
		if( BaseType ) {
			BaseType->GetMemberConstantsRecursive( OutMemberConstants );
		}
		for( auto const& MemberConstant : MemberConstants ) {
			OutMemberConstants.push_back( MemberConstant.get() );
		}
	}

	void StructTypeInfo::GetMemberVariablesRecursive( std::vector<MemberVariableInfo const*>& OutMemberVariables ) const {
		if( BaseType ) {
			BaseType->GetMemberVariablesRecursive( OutMemberVariables );
		}
		for( auto const& MemberVariable : MemberVariables ) {
			OutMemberVariables.push_back( MemberVariable.get() );
		}
	}
}
