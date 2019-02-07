#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	StructTypeInfo::StructTypeInfo( std::string_view InName, size_t InSize, size_t InAlignment, void (*Initializer)( StructTypeInfo* ) )
	: TypeInfo( StructTypeInfo::CLASSIFICATION, InName, InSize, InAlignment )
	{
		if( Initializer ) Initializer( this );
	}

	void StructTypeInfo::GetStaticConstantsRecursive( std::vector<StaticConstantInfo const*>& OutStaticConstants ) const {
		OutStaticConstants.reserve( OutStaticConstants.size() + StaticConstants.size() );
		for( auto const& StaticConstant : StaticConstants ) {
			OutStaticConstants.push_back( StaticConstant.get() );
		}
		if( BaseType ) BaseType->GetStaticConstantsRecursive( OutStaticConstants );
	}

	void StructTypeInfo::GetMemberConstantsRecursive( std::vector<MemberConstantInfo const*>& OutMemberConstants ) const {
		OutMemberConstants.reserve( OutMemberConstants.size() + MemberConstants.size() );
		for( auto const& MemberConstant : MemberConstants ) {
			OutMemberConstants.push_back( MemberConstant.get() );
		}
		if( BaseType ) BaseType->GetMemberConstantsRecursive( OutMemberConstants );
	}

	void StructTypeInfo::GetStaticVariablesRecursive( std::vector<StaticVariableInfo const*>& OutStaticVariables ) const {
		OutStaticVariables.reserve( OutStaticVariables.size() + StaticVariables.size() );
		for( auto const& StaticVariable : StaticVariables ) {
			OutStaticVariables.push_back( StaticVariable.get() );
		}
		if( BaseType ) BaseType->GetStaticVariablesRecursive( OutStaticVariables );
	}

	void StructTypeInfo::GetMemberVariablesRecursive( std::vector<MemberVariableInfo const*>& OutMemberVariables ) const {
		OutMemberVariables.reserve( OutMemberVariables.size() + MemberVariables.size() );
		for( auto const& MemberVariable : MemberVariables ) {
			OutMemberVariables.push_back( MemberVariable.get() );
		}
		if( BaseType ) BaseType->GetMemberVariablesRecursive( OutMemberVariables );
	}
}
