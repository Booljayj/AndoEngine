#include "Reflection/DynamicArrayTypeInfo.h"

namespace Reflection {
	DynamicArrayTypeInfo::DynamicArrayTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InInitializer, std::forward<std::string>( InName ), InSize )
	{}
}
