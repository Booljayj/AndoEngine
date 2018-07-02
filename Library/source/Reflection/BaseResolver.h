#pragma once
#include <string_view>
#include "Reflection/TypeInfo.h"

namespace Reflection
{
	/** Global reflection accessor type, specialized for types which are known to the reflection system */
	template<typename TTYPE>
	struct TypeResolver {
		static TypeInfo const* Get() { return nullptr; }
		static std::string_view GetName() { return "{{UNKNOWN}}"; }
	};
}