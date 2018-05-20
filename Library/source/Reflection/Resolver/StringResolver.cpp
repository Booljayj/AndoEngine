#include "Reflection/Resolver/StringResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	StringTypeInfo::StringTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
	: TypeInfo( InInitializer, std::forward<std::string>( InName ), InSize )
	{}
	int8_t StringTypeInfo::Compare( void const* A, void const* B ) const {
		return static_cast<std::string const*>( A )->compare( *static_cast<std::string const*>( B ) );
	}

	StringTypeInfo const TypeInfo__std_string{
		[]( TypeInfo* Info ) {
			Info->Description = "dynamic string";
			//Info->Serializer = make_unique<StringSerializer>();
		},
		"std::string", sizeof( std::string )
	};
}
