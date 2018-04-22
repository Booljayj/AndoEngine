#pragma once
#include <map>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TableTypeInfo.h"

namespace Reflection {
	template<typename TKEY, typename TVALUE>
	struct TypeResolver<std::map<TKEY, TVALUE>> {
		using TTABLE = std::map<TKEY, TVALUE>;
		using TTABLE_INFO = TTableTypeInfo<TKEY, TVALUE, TTABLE>;

		static TypeInfo* Get() {
			static TTABLE_INFO InstancedTypeInfo{
				[]( TypeInfo* Info ) {
					Info->Name = "std::map<K,V>";
					Info->Description = "ordered map";
					Info->Size = sizeof( std::map<TKEY, TVALUE> );
					//Info->Serializer = make_unique<TableSerializer>();

					if( TableTypeInfo* TableInfo = Info->As<TableTypeInfo>() ) {
						TableInfo->KeyType = TypeResolver<TKEY>::Get();
						TableInfo->ValueType = TypeResolver<TVALUE>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};
}
