#include "Reflection/FixedArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	FixedArrayTypeInfo::FixedArrayTypeInfo( void (*Initializer)( FixedArrayTypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, std::forward<std::string>( InName ), InSize )
	{
		if( Initializer ) Initializer( this );
	}

	std::string_view FixedArrayTypeInfo::GetName() const {
		static std::string FullName = MakeArrayName( ElementType, 0 );
		return FullName;
	}
}
