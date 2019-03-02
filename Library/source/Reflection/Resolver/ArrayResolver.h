#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Reflection/TypeResolver.h"
#include "Reflection/ArrayTypeInfo.h"

#define L_DYNAMIC_ARRAY_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TELEMENT>\
struct TypeResolver<__TEMPLATE__<TELEMENT>> {\
	static TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr sid_t GetID() { return id_combine( id( #__TEMPLATE__ ), TypeResolver<TELEMENT>::GetID() ); }\
};\
template<typename TELEMENT>\
TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const TypeResolver<__TEMPLATE__<TELEMENT>>::_TypeInfo{ __DESCRIPTION__, nullptr }

namespace Reflection {
	//============================================================
	// Standard fixed array type specializations

	template<typename TELEMENT, size_t SIZE>
	struct TypeResolver<std::array<TELEMENT, SIZE>> {
		static_assert( SIZE < std::numeric_limits<sid_t>::max(), "Fixed-size arrays larger than the capacity of a sid_t are not supported because by definition they cannot generate a unique id." );
		static TFixedArrayTypeInfo<TELEMENT, SIZE, std::array<TELEMENT, SIZE>> const _TypeInfo;
		static TypeInfo const* Get() { return &_TypeInfo; }
		static constexpr sid_t GetID() {
			const sid_t IDs[3] = { id( "std::array" ), TypeResolver<TELEMENT>::GetID(), static_cast<sid_t>( SIZE ) };
			return id_combine( IDs, 3 );
		}
	};
	template<typename TELEMENT, size_t SIZE>
	TFixedArrayTypeInfo<TELEMENT, SIZE, std::array<TELEMENT, SIZE>> const TypeResolver<std::array<TELEMENT, SIZE>>::_TypeInfo{ "fixed array", nullptr };

	//============================================================
	// Standard dynamic array type specializations

	L_DYNAMIC_ARRAY_RESOLVER( std::vector, "dynamic array" );
	L_DYNAMIC_ARRAY_RESOLVER( std::forward_list, "singly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::list, "doubly-linked list" );
	L_DYNAMIC_ARRAY_RESOLVER( std::deque, "double-ended queue" );
}

#undef L_DYNAMIC_ARRAY_RESOLVER
