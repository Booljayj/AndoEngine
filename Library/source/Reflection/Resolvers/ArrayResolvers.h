#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/ArrayTypeInfo.h"

#define L_DYNAMIC_ARRAY_RESOLVER( _ArrayTemplate_, _Description_ )\
template<typename ElementType>\
struct TypeResolver_Implementation<_ArrayTemplate_<ElementType>> {\
	static TDynamicArrayTypeInfo<ElementType, _ArrayTemplate_<ElementType>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #_ArrayTemplate_ } + TypeResolver<ElementType>::GetID(); }\
};\
template<typename ElementType>\
TDynamicArrayTypeInfo<ElementType, _ArrayTemplate_<ElementType>> const TypeResolver_Implementation<_ArrayTemplate_<ElementType>>::_TypeInfo{ _Description_, FTypeFlags::None, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard fixed array type specializations

		template<typename ElementType, size_t Size>
		struct TypeResolver_Implementation<std::array<ElementType, Size>> {
			static TFixedArrayTypeInfo<ElementType, Size, std::array<ElementType, Size>> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() {
				return Hash128{ "std::array" } + TypeResolver<ElementType>::GetID() + Hash128{ static_cast<uint64_t>( Size ) };
			}
		};
		template<typename ElementType, size_t Size>
		TFixedArrayTypeInfo<ElementType, Size, std::array<ElementType, Size>> const TypeResolver_Implementation<std::array<ElementType, Size>>::_TypeInfo{ "fixed array", FTypeFlags::None, nullptr };

		//============================================================
		// Standard dynamic array type specializations

		L_DYNAMIC_ARRAY_RESOLVER(std::vector, "dynamic array");
		L_DYNAMIC_ARRAY_RESOLVER(std::forward_list, "singly-linked list");
		L_DYNAMIC_ARRAY_RESOLVER(std::list, "doubly-linked list");
		L_DYNAMIC_ARRAY_RESOLVER(std::deque, "double-ended queue");
	}
}

#undef L_DYNAMIC_ARRAY_RESOLVER
