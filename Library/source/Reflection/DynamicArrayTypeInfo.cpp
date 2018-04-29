#include "Reflection/DynamicArrayTypeInfo.h"

namespace Reflection {
	void DynamicArrayTypeInfo::OnLoaded( bool bLoadDependencies ) {
		TypeInfo::OnLoaded( bLoadDependencies );
		if( bLoadDependencies ) {
			ElementType->Load( bLoadDependencies );
		}
	}
}
