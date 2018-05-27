#include "Reflection/DynamicArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	DynamicArrayTypeInfo::DynamicArrayTypeInfo( std::string_view InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InName, InSize )
	{}
}
