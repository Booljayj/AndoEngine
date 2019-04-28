#pragma once
#include <map>
#include <unordered_map>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/MapTypeInfo.h"

#define L_MAP_TYPE_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TKEY, typename TVALUE>\
struct TypeResolver_Implementation<__TEMPLATE__<TKEY, TVALUE>> {\
	static TMapTypeInfo<TKEY, TVALUE, __TEMPLATE__<TKEY, TVALUE>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #__TEMPLATE__ } + TypeResolver<TKEY>::GetID() + TypeResolver<TVALUE>::GetID(); }\
};\
template<typename TKEY, typename TVALUE>\
TMapTypeInfo<TKEY, TVALUE, __TEMPLATE__<TKEY, TVALUE>> const TypeResolver_Implementation<__TEMPLATE__<TKEY, TVALUE>>::_TypeInfo{ __DESCRIPTION__, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard map type specializations

		L_MAP_TYPE_RESOLVER( std::map, "ordered map" );
		L_MAP_TYPE_RESOLVER( std::unordered_map, "unordered map" );
	}
}

#undef L_MAP_TYPE_RESOLVER
