#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/DynamicArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

#define L_DYNAMIC_ARRAY_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TELEMENT>\
struct TypeResolver<__TEMPLATE__<TELEMENT>> {\
	static TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const InstancedTypeInfo;\
	static TypeInfo const* Get() { return &InstancedTypeInfo; }\
	static std::string_view GetName() {\
		static std::string const Name = MakeTemplateName<TELEMENT>( #__TEMPLATE__ );\
		return Name;\
	}\
};\
template<typename TELEMENT>\
TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const TypeResolver<__TEMPLATE__<TELEMENT>>::InstancedTypeInfo{ __DESCRIPTION__ }

namespace Reflection {
	//============================================================
	// Standard dynamic array type specializations

	L_DYNAMIC_ARRAY_RESOLVER( std::vector, "dynamic array" );
	L_DYNAMIC_ARRAY_RESOLVER( std::forward_list, "singly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::list, "doubly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::deque, "double-ended queue" );
}

#undef L_DYNAMIC_ARRAY_RESOLVER
