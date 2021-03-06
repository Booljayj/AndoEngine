#pragma once
#include "Engine/Hash.h"
#include "Engine/STL.h"
#include "Engine/Utility.h"
#include "Reflection/TypeInfo.h"

/** Declare a resolver implementation for a type. Must be placed in the Reflection::Internal namespace. */
#define DECLARE_RESOLVER(type)\
template<> struct TypeResolver_Implementation<type> {\
	static TypeInfo const* Get();\
	static constexpr Hash128 GetID() { return Hash128{ STRINGIFY(type) }; }\
}

/** Define a resolver implementation for a type. Must be placed in the Reflection::Internal namespace, and a TyepInfo instance with the name info_<name> must exist. */
#define DEFINE_RESOLVER(type, name) TypeInfo const* TypeResolver_Implementation<type>::Get() { return &info_ ## name; }

namespace Reflection {
	//The following setup ensures that specializations of TypeResolver are required to implement the concept of a TypeResolver
	// struct, and that this concept is global. If actual concepts are added to the standard, this can be greatly simplified
	// and just use that.

	namespace Internal {
		//Struct which provides the TypeResolver implementation for a particular type. Must be specialized for all resolvable types
		template<typename Type>
		struct TypeResolver_Implementation {
			//static TypeInfo const* Get() { return nullptr; }
			//static constexpr Hash128 GetID() { return Hash128{}; }
		};

		//Template which holds the concept of a TypeResolver and passes through to the implementation. This should not be specialized,
		//though the standard lacks any way to stop that from happening.
		template<typename Type>
		struct TypeResolver_Concept {
			static inline TypeInfo const* Get() { return TypeResolver_Implementation<Type>::Get(); }
			static inline constexpr Hash128 GetID() { return TypeResolver_Implementation<Type>::GetID(); }
		};
	}

	//The primary TypeResolver template used externally. The using statement ensures that this cannot be specialized, and always
	// uses the correct concept and implementation structure.

	/** Global reflection accessor type */
	template<typename Type> using TypeResolver = Internal::TypeResolver_Concept<Type>;
}
