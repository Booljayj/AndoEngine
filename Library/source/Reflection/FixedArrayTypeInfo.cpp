#include "Reflection/FixedArrayTypeInfo.h"

namespace Reflection {
	FixedArrayTypeInfo::FixedArrayTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InInitializer, std::forward<std::string>( InName ), InSize )
	{}
}
