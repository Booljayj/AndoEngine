#include "Reflection/FixedArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	FixedArrayTypeInfo::FixedArrayTypeInfo( std::string_view InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InName, InSize )
	{}
}
