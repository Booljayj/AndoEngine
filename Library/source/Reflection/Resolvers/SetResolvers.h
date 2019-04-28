#include <set>
#include <unordered_set>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/SetTypeInfo.h"

#define L_SET_TYPE_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TVALUE>\
struct TypeResolver_Implementation<__TEMPLATE__<TVALUE>> {\
	static TSetTypeInfo<TVALUE, __TEMPLATE__<TVALUE>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #__TEMPLATE__ } + TypeResolver<TVALUE>::GetID(); }\
};\
template<typename TVALUE>\
TSetTypeInfo<TVALUE, __TEMPLATE__<TVALUE>> const TypeResolver_Implementation<__TEMPLATE__<TVALUE>>::_TypeInfo{ __DESCRIPTION__, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard set type specializations

		L_SET_TYPE_RESOLVER( std::set, "ordered set" );
		L_SET_TYPE_RESOLVER( std::unordered_set, "unordered set" );
	}
}

#undef L_SET_TYPE_RESOLVER
