#include "Reflection/DynamicArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	DynamicArrayTypeInfo::DynamicArrayTypeInfo( void (*Initializer)( DynamicArrayTypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, std::forward<std::string>( InName ), InSize )
	{
		if( Initializer ) Initializer( this );
	}

	std::string_view DynamicArrayTypeInfo::GetName() const {
		static std::string FullName = MakeTemplateName( Name, { ElementType } );
		return FullName;
	}
}
