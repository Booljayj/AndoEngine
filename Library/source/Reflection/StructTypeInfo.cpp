#include "Reflection/StructTypeInfo.h"
#include "Reflection/Resolver.h"

namespace Reflection {
	StructTypeInfo::StructTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InInitializer, std::forward<std::string>( InName ), InSize )
	{}

	void StructTypeInfo::OnLoaded() {
		TypeInfo::OnLoaded();

		//ensures that arrays are sorted based on NameHash for quick lookups.
		auto ByNameHash = []( auto const& A, auto const& B) {
			return A->NameHash < B->NameHash;
		};
		std::sort( StaticConstants.begin(), StaticConstants.end(), ByNameHash );
		std::sort( MemberConstants.begin(), MemberConstants.end(), ByNameHash );
		std::sort( StaticVariables.begin(), StaticVariables.end(), ByNameHash );
		std::sort( MemberVariables.begin(), MemberVariables.end(), ByNameHash );
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
