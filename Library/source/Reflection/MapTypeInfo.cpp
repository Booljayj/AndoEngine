#include "Reflection/MapTypeInfo.h"

namespace Reflection {
	void MapTypeInfo::OnLoaded( bool bLoadDependencies ) {
		TypeInfo::OnLoaded( bLoadDependencies );
		if( bLoadDependencies ) {
			KeyType->Load( bLoadDependencies );
			ValueType->Load( bLoadDependencies );
		}
	}
}
