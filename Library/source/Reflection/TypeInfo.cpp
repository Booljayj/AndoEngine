#include "Reflection/TypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	std::deque<TypeInfo const*> TypeInfo::GlobalTypeCollection{};

	TypeInfo const* TypeInfo::FindTypeByNameHash( uint32_t NameHash )
	{
		for( TypeInfo const* Info : GlobalTypeCollection ) {
			if( Info && Info->NameHash == NameHash ) {
				return Info;
			}
		}
		return nullptr;
	}
	TypeInfo const* TypeInfo::FindTypeByName( std::string_view Name )
	{
		for( TypeInfo const* Info : GlobalTypeCollection ) {
			if( Info && Info->Name == Name ) {
				return Info;
			}
		}
		return nullptr;
	}

	TypeInfo::TypeInfo( ETypeClassification InClassification, std::string_view InName, size_t InSize )
		: Classification( InClassification )
		, Name( InName )
		, NameHash( id( InName ) )
		, Size( InSize )
	{
		GlobalTypeCollection.push_back( this );
	}

	int8_t TypeInfo::Compare( void const* A, void const* B ) const {
		return memcmp( A, B, Size );
	}
}
