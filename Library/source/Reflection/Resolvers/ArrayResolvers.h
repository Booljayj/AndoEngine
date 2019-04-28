#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/ArrayTypeInfo.h"

#define L_DYNAMIC_ARRAY_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TELEMENT>\
struct TypeResolver_Implementation<__TEMPLATE__<TELEMENT>> {\
	static TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #__TEMPLATE__ } + TypeResolver<TELEMENT>::GetID(); }\
};\
template<typename TELEMENT>\
TDynamicArrayTypeInfo<TELEMENT, __TEMPLATE__<TELEMENT>> const TypeResolver_Implementation<__TEMPLATE__<TELEMENT>>::_TypeInfo{ __DESCRIPTION__, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard fixed array type specializations

		template<typename TELEMENT, size_t SIZE>
		struct TypeResolver_Implementation<std::array<TELEMENT, SIZE>> {
			static TFixedArrayTypeInfo<TELEMENT, SIZE, std::array<TELEMENT, SIZE>> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() {
				return Hash128{ "std::array" } + TypeResolver<TELEMENT>::GetID() + Hash128{ static_cast<uint64_t>( SIZE ) };
			}
		};
		template<typename TELEMENT, size_t SIZE>
		TFixedArrayTypeInfo<TELEMENT, SIZE, std::array<TELEMENT, SIZE>> const TypeResolver_Implementation<std::array<TELEMENT, SIZE>>::_TypeInfo{ "fixed array", nullptr };

		//============================================================
		// Standard dynamic array type specializations

		L_DYNAMIC_ARRAY_RESOLVER( std::vector, "dynamic array" );
		L_DYNAMIC_ARRAY_RESOLVER( std::forward_list, "singly-linked list" );
		L_DYNAMIC_ARRAY_RESOLVER( std::list, "doubly-linked list" );
		L_DYNAMIC_ARRAY_RESOLVER( std::deque, "double-ended queue" );
	}
}

#undef L_DYNAMIC_ARRAY_RESOLVER
