#pragma once
#include <string_view>
#include "Engine/StringID.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	/** Global reflection accessor type, specialized for types which are known to the reflection system */
	template<typename TTYPE>
	struct TypeResolver {
		//static TypeInfo const* Get() { return nullptr; }
		//static constexpr sid_t GetID() { return 0; }
	};
}
