#pragma once
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	//Strings are a special type of primitive that is handled differently due to dynamic memory allocations
	struct StringTypeInfo : public TypeInfo {
		StringTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize );
		virtual int8_t Compare( void const* A, void const* B ) const override;
	};

	extern StringTypeInfo const TypeInfo__std_string;
	template<> struct TypeResolver<std::string> {
		static TypeInfo const* Get() { return &TypeInfo__std_string; }
	};
}
