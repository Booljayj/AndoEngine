#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/ArrayTypeInfo.h"
#include "Serialization/ArraySerializer.h"

#define L_DYNAMIC_ARRAY_RESOLVER(ARRAY_TEMPLATE, DESCRIPTION)\
template<typename ElementType>\
struct TypeResolver_Implementation<ARRAY_TEMPLATE<ElementType>> {\
	static TDynamicArrayTypeInfo<ARRAY_TEMPLATE<ElementType>, ElementType> const typeInfo;\
	static TypeInfo const* Get() { return &typeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #ARRAY_TEMPLATE } + TypeResolver<ElementType>::GetID(); }\
};\
template<typename ElementType>\
TDynamicArrayTypeInfo<ARRAY_TEMPLATE<ElementType>, ElementType> const TypeResolver_Implementation<ARRAY_TEMPLATE<ElementType>>::typeInfo{ DESCRIPTION, FTypeFlags::None, &Serialization::defaultArraySerializer }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard fixed array type specializations

		template<typename ElementType, size_t Size>
		struct TypeResolver_Implementation<std::array<ElementType, Size>> {
			static TFixedArrayTypeInfo<std::array<ElementType, Size>, ElementType, Size> const typeInfo;
			static TypeInfo const* Get() { return &typeInfo; }
			static constexpr Hash128 GetID() {
				return Hash128{ "std::array" } + TypeResolver<ElementType>::GetID() + Hash128{ static_cast<uint64_t>( Size ) };
			}
		};
		template<typename ElementType, size_t Size>
		TFixedArrayTypeInfo<std::array<ElementType, Size>, ElementType, Size> const TypeResolver_Implementation<std::array<ElementType, Size>>::typeInfo{ "fixed array", FTypeFlags::None, &Serialization::defaultArraySerializer };

		//============================================================
		// Standard dynamic array type specializations

		L_DYNAMIC_ARRAY_RESOLVER(std::vector, "dynamic array");
		L_DYNAMIC_ARRAY_RESOLVER(std::forward_list, "singly-linked list");
		L_DYNAMIC_ARRAY_RESOLVER(std::list, "doubly-linked list");
		L_DYNAMIC_ARRAY_RESOLVER(std::deque, "double-ended queue");
	}
}

#undef L_DYNAMIC_ARRAY_RESOLVER
