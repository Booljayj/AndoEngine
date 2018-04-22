#pragma once

namespace Reflection
{
	struct TypeInfo;

	//Global reflection accessor type, specialized for different types
	template<typename TTYPE>
	struct TypeResolver {
		static TypeInfo* Get() { return TTYPE::GetTypeInfo(); }
	};
}
