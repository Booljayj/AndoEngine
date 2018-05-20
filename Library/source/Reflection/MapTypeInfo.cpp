#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	MapTypeInfo::MapTypeInfo( void (*Initializer)( MapTypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, std::forward<std::string>( InName ), InSize )
	{
		if( Initializer ) Initializer( this );
	}

	std::string_view MapTypeInfo::GetName() const {
		static std::string FullName = MakeTemplateName( Name, { KeyType, ValueType } );
		return FullName;
	}
}