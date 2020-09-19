#pragma once
#include <set>
#include <unordered_set>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/SetTypeInfo.h"

#define L_SET_TYPE_RESOLVER(SetTemplate, DescriptionString)\
template<typename ValueType>\
struct TypeResolver_Implementation<SetTemplate<ValueType>> {\
	static TSetTypeInfo<SetTemplate<ValueType>, ValueType> const typeInfo;\
	static TypeInfo const* Get() { return &typeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #SetTemplate } + TypeResolver<ValueType>::GetID(); }\
};\
template<typename ValueType>\
TSetTypeInfo<SetTemplate<ValueType>, ValueType> const TypeResolver_Implementation<SetTemplate<ValueType>>::typeInfo = TSetTypeInfo<SetTemplate<ValueType>, ValueType>{}\
	.Description(DescriptionString)

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard set type specializations

		L_SET_TYPE_RESOLVER(std::set, "ordered set");
		L_SET_TYPE_RESOLVER(std::unordered_set, "unordered set");
	}
}

#undef L_SET_TYPE_RESOLVER
