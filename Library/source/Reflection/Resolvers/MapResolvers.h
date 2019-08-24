#pragma once
#include <map>
#include <unordered_map>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/MapTypeInfo.h"

#define L_MAP_TYPE_RESOLVER( _MapTemplate_, _Description_ )\
template<typename KeyType, typename ValueType>\
struct TypeResolver_Implementation<_MapTemplate_<KeyType, ValueType>> {\
	static TMapTypeInfo<KeyType, ValueType, _MapTemplate_<KeyType, ValueType>> const _TypeInfo;\
	static TypeInfo const* Get() { return &_TypeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #_MapTemplate_ } + TypeResolver<KeyType>::GetID() + TypeResolver<ValueType>::GetID(); }\
};\
template<typename KeyType, typename ValueType>\
TMapTypeInfo<KeyType, ValueType, _MapTemplate_<KeyType, ValueType>> const TypeResolver_Implementation<_MapTemplate_<KeyType, ValueType>>::_TypeInfo{ _Description_, FTypeFlags::None, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard map type specializations

		L_MAP_TYPE_RESOLVER(std::map, "ordered map");
		L_MAP_TYPE_RESOLVER(std::unordered_map, "unordered map");
	}
}

#undef L_MAP_TYPE_RESOLVER
