#include "Reflection/Resolver/StringResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	StructTypeInfo TypeInfo__std_string{
		[]( TypeInfo* Info ) {
			Info->Description = "dynamic string";
			Info->Compare = []( TypeInfo* Info, void const* A, void const* B ) -> int8_t {
				return static_cast<std::string const*>( A )->compare( *static_cast<std::string const*>( B ) );
			};

			if( auto* StructInfo = Info->As<StructTypeInfo>() ) {
				//@todo Fill out function info for std::string
			}
		},
		"std::string", sizeof( std::string )
	};
}
