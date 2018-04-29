#include "Reflection/Resolver/StringResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	StructTypeInfo TypeInfo__std_string{
		"std::string",
		sizeof( std::string ),
		[]( TypeInfo* Info ) {
			Info->Description = "dynamic string";
			if( auto* StructInfo = Info->As<StructTypeInfo>() ) {
				//@todo Fill out function info for std::string
			}
		}
	};
}
