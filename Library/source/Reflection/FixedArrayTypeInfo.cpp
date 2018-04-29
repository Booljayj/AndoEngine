#include "Reflection/FixedArrayTypeInfo.h"

namespace Reflection {
	void FixedArrayTypeInfo::OnLoaded( bool bLoadDependencies ) {
		TypeInfo::OnLoaded( bLoadDependencies );
		if( bLoadDependencies ) {
			ElementType->Load( bLoadDependencies );
		}
	}
}
