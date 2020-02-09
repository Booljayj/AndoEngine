#pragma once
#include <map>
#include <unordered_map>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/MapTypeInfo.h"

#define L_MAP_TYPE_RESOLVER(MapTemplate, Description)\
template<typename KeyType, typename ValueType>\
struct TypeResolver_Implementation<MapTemplate<KeyType, ValueType>> {\
	static TMapTypeInfo<MapTemplate<KeyType, ValueType>, KeyType, ValueType> const typeInfo;\
	static TypeInfo const* Get() { return &typeInfo; }\
	static constexpr Hash128 GetID() { return Hash128{ #MapTemplate } + TypeResolver<KeyType>::GetID() + TypeResolver<ValueType>::GetID(); }\
};\
template<typename KeyType, typename ValueType>\
TMapTypeInfo<MapTemplate<KeyType, ValueType>, KeyType, ValueType> const TypeResolver_Implementation<MapTemplate<KeyType, ValueType>>::typeInfo{ Description, FTypeFlags::None, nullptr }

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard map type specializations

		L_MAP_TYPE_RESOLVER(std::map, "ordered map");
		L_MAP_TYPE_RESOLVER(std::unordered_map, "unordered map");
	}
}

#undef L_MAP_TYPE_RESOLVER
