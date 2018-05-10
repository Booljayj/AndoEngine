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
}
