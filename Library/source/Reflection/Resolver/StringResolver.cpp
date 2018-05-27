#include "Reflection/Resolver/StringResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	StringTypeInfo::StringTypeInfo( void (*Initializer)( StringTypeInfo* ) )
	: TypeInfo( TypeInfo::CLASSIFICATION, TypeResolver<std::string>::GetName(), sizeof( std::string ) )
	{
		if( Initializer ) Initializer( this );
	}
	int8_t StringTypeInfo::Compare( void const* A, void const* B ) const {
		return static_cast<std::string const*>( A )->compare( *static_cast<std::string const*>( B ) );
	}

	StringTypeInfo const TypeInfo__std_string{
		[]( StringTypeInfo* Info ) {
			Info->Description = "dynamic string";
			//Info->Serializer = make_unique<StringSerializer>();
		}
	};
}
