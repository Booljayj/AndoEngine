#include <set>
#include <unordered_set>
#include "Reflection/TypeResolver.h"
#include "Reflection/SetTypeInfo.h"

#define L_SET_TYPE_RESOLVER( __TEMPLATE__, __DESCRIPTION__ )\
template<typename TVALUE>\
struct TypeResolver<__TEMPLATE__<TVALUE>> {\
	static TSetTypeInfo<TVALUE, __TEMPLATE__<TVALUE>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr sid_t GetID() {\
		const sid_t IDs[2] = { id( #__TEMPLATE__ ), TypeResolver<TVALUE>::GetID() };\
		return id_combine( IDs, 2 );\
	}\
};\
template<typename TVALUE>\
TSetTypeInfo<TVALUE, __TEMPLATE__<TVALUE>> const TypeResolver<__TEMPLATE__<TVALUE>>::_TypeInfo{ __DESCRIPTION__, nullptr }

namespace Reflection {
	//============================================================
	// Standard set type specializations

	L_SET_TYPE_RESOLVER( std::set, "ordered set" );
	L_SET_TYPE_RESOLVER( std::unordered_set, "unordered set" );
}

#undef L_SET_TYPE_RESOLVER
