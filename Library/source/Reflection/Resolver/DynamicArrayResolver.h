#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/DynamicArrayTypeInfo.h"

#define L_DYNAMIC_ARRAY_RESOLVER( __TEMPLATE__, __NAME__, __DESCRIPTION__ )\
template<typename TELEMENT>\
struct TypeResolver<__TEMPLATE__<TELEMENT>> {\
	static TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const InstancedTypeInfo;\
	static TypeInfo const* Get() { return &InstancedTypeInfo; }\
};\
template<typename TELEMENT>\
TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const TypeResolver<__TEMPLATE__<TELEMENT>>::InstancedTypeInfo{\
	[]( DynamicArrayTypeInfo* DynamicArrayInfo ) {\
		DynamicArrayInfo->Description = __DESCRIPTION__;\
		DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();\
	},\
	__NAME__, sizeof( __TEMPLATE__<TELEMENT> )\
}

namespace Reflection {
	//============================================================
	// Standard dynamic array type specializations

	L_DYNAMIC_ARRAY_RESOLVER( std::vector, "std::vector", "dynamic array" );
	L_DYNAMIC_ARRAY_RESOLVER( std::forward_list, "std::forward_list", "singly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::list, "std::list", "doubly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::deque, "std::deque", "double-ended queue" );
}

#undef L_DYNAMIC_ARRAY_RESOLVER
