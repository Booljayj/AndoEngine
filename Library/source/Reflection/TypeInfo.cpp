#include "Reflection/TypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	void TypeInfo::OnLoaded( bool bLoadDependencies /*= true */ )
	{
		if( Name.empty() ) {
			Name = "UNDEFINED";
		}
		NameHash = id( Name.c_str() );
	}

	void TypeInfo::Load( bool bLoadDependencies /*= true */ )
	{
		if( !bIsLoaded ) {
			if( !!Initializer ) Initializer( this );
			OnLoaded( bLoadDependencies );
			bIsLoaded = true;
		}
	}
}
