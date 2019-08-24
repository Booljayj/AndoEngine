#pragma once
#include <set>
#include <unordered_set>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/SetTypeInfo.h"

#define L_SET_TYPE_RESOLVER( _SetTemplate_, _Description_ )\
template<typename ValueType>\
struct TypeResolver_Implementation<_SetTemplate_<ValueType>> {\
	static TSetTypeInfo<ValueType, _SetTemplate_<ValueType>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #_SetTemplate_ } + TypeResolver<ValueType>::GetID(); }\
};\
template<typename ValueType>\
TSetTypeInfo<ValueType, _SetTemplate_<ValueType>> const TypeResolver_Implementation<_SetTemplate_<ValueType>>::_TypeInfo{ _Description_, FTypeFlags::None, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard set type specializations

		L_SET_TYPE_RESOLVER(std::set, "ordered set");
		L_SET_TYPE_RESOLVER(std::unordered_set, "unordered set");
	}
}

#undef L_SET_TYPE_RESOLVER
