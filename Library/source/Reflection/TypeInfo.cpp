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

	TypeInfo::TypeInfo( ETypeClassification InClassification, std::string_view InName, size_t InSize, std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer )
		: Classification( InClassification )
		, Name( InName )
		, NameHash( id( InName ) )
		, Size( InSize )
		, Description( InDescription )
		, Flags( InFlags )
		, Serializer( InSerializer )
	{
		GlobalTypeCollection.push_back( this );
	}
	TypeInfo::TypeInfo( ETypeClassification InClassification, std::string_view InName, size_t InSize )
		: TypeInfo( InClassification, InName, InSize, "", (FTypeFlags)0, nullptr )
	{}

	int8_t TypeInfo::Compare( void const* A, void const* B ) const {
		return memcmp( A, B, Size );
	}
}
