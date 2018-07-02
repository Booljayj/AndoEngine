#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Reflection/BaseResolver.h"
#include "Reflection/ArrayTypeInfo.h"
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
	// Standard fixed array type specializations

	template<typename TELEMENT, size_t SIZE>
	struct TypeResolver<std::array<TELEMENT, SIZE>> {
		static_assert( SIZE < UINT32_MAX, "Fixed-size arrays larger than the capacity of a uint32 are not supported" );
		static TFixedArrayTypeInfo<TELEMENT, SIZE, std::array<TELEMENT, SIZE>> const InstancedTypeInfo;
		static TypeInfo const* Get() { return &InstancedTypeInfo; }
		static std::string_view GetName() {
			static std::string const Name = MakeTemplateName<TELEMENT, std::integral_constant<size_t, SIZE>>( "std::array" );
			return Name;
		}
	};
	template<typename TELEMENT, size_t SIZE>
	TFixedArrayTypeInfo<TELEMENT, SIZE, std::array<TELEMENT, SIZE>> const TypeResolver<std::array<TELEMENT, SIZE>>::InstancedTypeInfo{ "fixed array" };

	//============================================================
	// Standard dynamic array type specializations

	L_DYNAMIC_ARRAY_RESOLVER( std::vector, "dynamic array" );
	L_DYNAMIC_ARRAY_RESOLVER( std::forward_list, "singly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::list, "doubly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::deque, "double-ended queue" );
}

#undef L_DYNAMIC_ARRAY_RESOLVER