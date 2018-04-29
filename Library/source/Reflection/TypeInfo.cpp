#include "Reflection/TypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	void TypeInfo::OnLoaded( bool bLoadDependencies /*= true */ )
	{
		NameHash = id( Name );
	}

	void TypeInfo::Load( bool bLoadDependencies /*= true */ )
	{
		if( !bIsLoaded ) {
			bIsLoaded = true;
			if( !!Initializer ) Initializer( this );
			OnLoaded( bLoadDependencies );
		}
	}
}
