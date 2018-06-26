#pragma once
#include <map>
#include <unordered_map>
#include "Reflection/BaseResolver.h"
#include "Reflection/MapTypeInfo.h"
#include "Reflection/TypeUtility.h"

#define L_MAP_TYPE_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TKEY, typename TVALUE>\
struct TypeResolver<__TEMPLATE__<TKEY, TVALUE>> {\
	static TMapTypeInfo<TKEY, TVALUE, __TEMPLATE__<TKEY, TVALUE>> const InstancedTypeInfo;\
	static TypeInfo const* Get() { return &InstancedTypeInfo; }\
	static std::string_view GetName() {\
		static std::string const Name = MakeTemplateName<TKEY, TVALUE>( #__TEMPLATE__ );\
		return Name;\
	}\
};\
template<typename TKEY, typename TVALUE>\
TMapTypeInfo<TKEY, TVALUE, __TEMPLATE__<TKEY, TVALUE>> const TypeResolver<__TEMPLATE__<TKEY, TVALUE>>::InstancedTypeInfo{ __DESCRIPTION__ }

namespace Reflection {
	//============================================================
	// Standard map type specializations

	L_MAP_TYPE_RESOLVER( std::map, "ordered map" );
	L_MAP_TYPE_RESOLVER( std::unordered_map, "unordered map" );
}

#undef L_MAP_TYPE_RESOLVER
