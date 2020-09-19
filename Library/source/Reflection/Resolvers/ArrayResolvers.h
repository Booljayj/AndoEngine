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

#define L_DYNAMIC_ARRAY_RESOLVER(ArrayTemplate, DescriptionString)\
template<typename ElementType>\
struct TypeResolver_Implementation<ArrayTemplate<ElementType>> {\
	static TDynamicArrayTypeInfo<ArrayTemplate<ElementType>, ElementType> const typeInfo;\
	static TypeInfo const* Get() { return &typeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #ArrayTemplate } + TypeResolver<ElementType>::GetID(); }\
};\
template<typename ElementType>\
TDynamicArrayTypeInfo<ArrayTemplate<ElementType>, ElementType> const TypeResolver_Implementation<ArrayTemplate<ElementType>>::typeInfo = TDynamicArrayTypeInfo<ArrayTemplate<ElementType>, ElementType>{}\
	.Description(DescriptionString).Serializer(&Serialization::defaultArraySerializer)

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
		TFixedArrayTypeInfo<std::array<ElementType, Size>, ElementType, Size> const TypeResolver_Implementation<std::array<ElementType, Size>>::typeInfo = TFixedArrayTypeInfo<std::array<ElementType, Size>, ElementType, Size>{}
			.Description("fixed array").Serializer(&Serialization::defaultArraySerializer);

		//============================================================
		// Standard dynamic array type specializations

		L_DYNAMIC_ARRAY_RESOLVER(std::vector, "dynamic array");
		L_DYNAMIC_ARRAY_RESOLVER(std::forward_list, "singly-linked list");
		L_DYNAMIC_ARRAY_RESOLVER(std::list, "doubly-linked list");
		L_DYNAMIC_ARRAY_RESOLVER(std::deque, "double-ended queue");
	}
}

#undef L_DYNAMIC_ARRAY_RESOLVER
