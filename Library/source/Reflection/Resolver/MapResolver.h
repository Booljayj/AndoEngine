#pragma once
#include <map>
#include <unordered_map>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/MapTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard map type specializations

	template<typename TKEY, typename TVALUE>
	struct TypeResolver<std::map<TKEY, TVALUE>> {
		using TMAP = std::map<TKEY, TVALUE>;
		using TMAP_INFO = TMapTypeInfo<TKEY, TVALUE, TMAP>;

		static TypeInfo* Get() {
			static TMAP_INFO InstancedTypeInfo{
				"std::map",
				sizeof( TMAP ),
				[]( TypeInfo* Info ) {
					Info->Description = "ordered map";
					//Info->Serializer = make_unique<TableSerializer>();

					if( auto* MapInfo = Info->As<MapTypeInfo>() ) {
						MapInfo->KeyType = TypeResolver<TKEY>::Get();
						MapInfo->ValueType = TypeResolver<TVALUE>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};

	template<typename TKEY, typename TVALUE>
	struct TypeResolver<std::unordered_map<TKEY, TVALUE>> {
		using TMAP = std::unordered_map<TKEY, TVALUE>;
		using TMAP_INFO = TMapTypeInfo<TKEY, TVALUE, TMAP>;

		static TypeInfo* Get() {
			static TMAP_INFO InstancedTypeInfo{
				"std::unordered_map",
				sizeof( TMAP ),
				[]( TypeInfo* Info ) {
					Info->Description = "unordered map";
					//Info->Serializer = make_unique<TableSerializer>();

					if( auto* MapInfo = Info->As<MapTypeInfo>() ) {
						MapInfo->KeyType = TypeResolver<TKEY>::Get();
						MapInfo->ValueType = TypeResolver<TVALUE>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};
}
