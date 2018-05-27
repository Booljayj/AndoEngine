#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo( std::string_view InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InName, InSize )
	{}
}