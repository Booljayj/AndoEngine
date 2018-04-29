#pragma once
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	extern StructTypeInfo TypeInfo__std_string;
	template<> struct TypeResolver<std::string> {
		static TypeInfo* Get() { return &TypeInfo__std_string; }
	};
}
