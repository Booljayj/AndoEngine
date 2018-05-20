#pragma once
#include "Reflection/TypeInfo.h"

namespace Reflection
{
	struct TypeInfo;

	/** Global reflection accessor type, specialized for different types */
	template<typename TTYPE>
	struct TypeResolver {
		static TypeInfo const* Get() { return &TTYPE::__TypeInfo__; }
	};
}
