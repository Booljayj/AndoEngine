#include "Reflection/TypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	TypeInfo::TypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InInitializer, std::forward<std::string>( InName ), InSize )
	{}

	TypeInfo::TypeInfo( ETypeClassification InClassification, void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: Classification( InClassification )
		, Initializer( InInitializer )
		, Name( InName )
		, NameHash( id( InName.data() ) )
		, Size( InSize )
	{
		Compare = &DefaultCompare;
	}

	int8_t TypeInfo::DefaultCompare( TypeInfo* Info, void const* A, void const* B ) {
		return memcmp( A, B, Info->GetSize() );
	}

	void TypeInfo::Load() {
		if( !bIsLoaded ) {
			bIsLoaded = true;
			if( !!Initializer ) Initializer( this );
			OnLoaded();
		}
	}

	void TypeInfo::OnLoaded() {}
}
