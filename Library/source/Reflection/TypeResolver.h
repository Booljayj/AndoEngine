#pragma once
#include "Engine/Hash.h"
#include "Engine/STL.h"
#include "Engine/Utility.h"
#include "Reflection/TypeInfo.h"

/** Declare a resolver implementation for a type. Must be placed in the Reflection namespace. */
#define DECLARE_RESOLVER(type)\
template<> struct TypeResolver<type> {\
	static TypeInfo const* Get();\
	static constexpr Hash128 GetID() { return Hash128{ STRINGIFY(type) }; }\
}

/** Define a resolver implementation for a type. Must be placed in the Reflection::Internal namespace, and a TyepInfo instance with the name info_<name> must exist. */
#define DEFINE_RESOLVER(type, name) TypeInfo const* TypeResolver<type>::Get() { return &info_ ## name; }

namespace Reflection {
	//Struct which provides reflection information for a particular type. Must be specialized for all resolvable types
	template<typename Type>
	struct TypeResolver {
		static TypeInfo const* Get() { return nullptr; }
		static constexpr Hash128 GetID() { return Hash128{}; }
	};
}
