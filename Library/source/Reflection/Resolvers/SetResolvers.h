#pragma once
#include <set>
#include <unordered_set>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/SetTypeInfo.h"

#define L_SET_TYPE_RESOLVER(SET_TEMPLATE, DESCRIPTION)\
template<typename ValueType>\
struct TypeResolver_Implementation<SET_TEMPLATE<ValueType>> {\
	static TSetTypeInfo<SET_TEMPLATE<ValueType>, ValueType> const typeInfo;\
	static TypeInfo const* Get() { return &typeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #SET_TEMPLATE } + TypeResolver<ValueType>::GetID(); }\
};\
template<typename ValueType>\
TSetTypeInfo<SET_TEMPLATE<ValueType>, ValueType> const TypeResolver_Implementation<SET_TEMPLATE<ValueType>>::typeInfo{ DESCRIPTION, FTypeFlags::None, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard set type specializations

		L_SET_TYPE_RESOLVER(std::set, "ordered set");
		L_SET_TYPE_RESOLVER(std::unordered_set, "unordered set");
	}
}

#undef L_SET_TYPE_RESOLVER
