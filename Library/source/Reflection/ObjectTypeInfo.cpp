#include "Reflection/ObjectTypeInfo.h"
#include "Reflection/Resolver.h"

namespace Reflection {
	void ObjectTypeInfo::OnLoaded( bool bLoadDependencies )
	{
		TypeInfo::OnLoaded( bLoadDependencies );

		//ensures that arrays are sorted based on NameHash for quick lookups.
		auto ByNameHash = []( auto const& A, auto const& B) {
			return A->NameHash < B->NameHash;
		};
		std::sort( StaticConstants.begin(), StaticConstants.end(), ByNameHash );
		std::sort( MemberConstants.begin(), MemberConstants.end(), ByNameHash );
		std::sort( StaticVariables.begin(), StaticVariables.end(), ByNameHash );
		std::sort( MemberVariables.begin(), MemberVariables.end(), ByNameHash );

		if( bLoadDependencies ) {
			for( auto const& StaticConstant : StaticConstants ) {
				StaticConstant->Type->Load( bLoadDependencies );
			}
			for( auto const& MemberConstant : MemberConstants ) {
				MemberConstant->Type->Load( bLoadDependencies );
			}
			for( auto const& StaticVariable : StaticVariables ) {
				StaticVariable->Type->Load( bLoadDependencies );
			}
			for( auto const& MemberVariable : MemberVariables ) {
				MemberVariable->Type->Load( bLoadDependencies );
			}
		}
	}
}