#pragma once
#include <map>
#include <unordered_map>
#include "Reflection/MapTypeInfo.h"

#define L_MAP_TYPE_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TKEY, typename TVALUE>\
struct TypeResolver<__TEMPLATE__<TKEY, TVALUE>> {\
	static TMapTypeInfo<TKEY, TVALUE, __TEMPLATE__<TKEY, TVALUE>> const InstancedTypeInfo;\
	static TypeInfo const* Get() { return &InstancedTypeInfo; }\
	static constexpr sid_t GetID() {\
		const sid_t IDs[3] = { id( #__TEMPLATE__ ), TypeResolver<TKEY>::GetID(), TypeResolver<TVALUE>::GetID() };\
		return id_combine( IDs, 3 );\
	}\
};\
template<typename TKEY, typename TVALUE>\
TMapTypeInfo<TKEY, TVALUE, __TEMPLATE__<TKEY, TVALUE>> const TypeResolver<__TEMPLATE__<TKEY, TVALUE>>::InstancedTypeInfo{ __DESCRIPTION__, nullptr }

namespace Reflection {
	//============================================================
	// Standard map type specializations

	L_MAP_TYPE_RESOLVER( std::map, "ordered map" );
	L_MAP_TYPE_RESOLVER( std::unordered_map, "unordered map" );
}

#undef L_MAP_TYPE_RESOLVER
