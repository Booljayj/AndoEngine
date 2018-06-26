#include "Reflection/StringTypeInfo.h"
#include "Reflection/Resolver/PrimitiveResolver.h"

namespace Reflection {
	StringTypeInfo::StringTypeInfo()
	: TypeInfo(
		TypeInfo::CLASSIFICATION,
		TypeResolver<std::string>::GetName(), sizeof( std::string ), "dynamic string",
		FTypeFlags::None, nullptr
	)
	{}

	int8_t StringTypeInfo::Compare( void const* A, void const* B ) const {
		return static_cast<std::string const*>( A )->compare( *static_cast<std::string const*>( B ) );
	}
}
