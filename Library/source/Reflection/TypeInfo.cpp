#include "Reflection/TypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	TypeInfo::TypeInfo( void (*Initializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, std::forward<std::string>( InName ), InSize )
	{
		if( Initializer ) Initializer( this );
	}

	TypeInfo::TypeInfo( ETypeClassification InClassification, std::string&& InName, size_t InSize )
		: Classification( InClassification )
		, Name( InName )
		, NameHash( id( InName.data() ) )
		, Size( InSize )
	{}

	int8_t TypeInfo::Compare( void const* A, void const* B ) const {
		return memcmp( A, B, Size );
	}
}
