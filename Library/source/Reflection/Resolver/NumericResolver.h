#pragma once
#include <string>
#include "Reflection/Resolver/TypeResolver.h"

namespace Reflection {
	template<typename T, size_t SIZE>
	struct TypeResolver<std::integral_constant<T, SIZE>> {
		static std::string_view GetName() {
			static std::string const Name = std::to_string( SIZE );
			return Name;
		}
	};
}
