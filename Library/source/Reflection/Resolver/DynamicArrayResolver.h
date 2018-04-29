#pragma once
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/DynamicArrayTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard dynamic array type specializations

	template<typename TELEMENT>
	struct TypeResolver<std::vector<TELEMENT>> {
		using TVECTOR = std::vector<TELEMENT>;
		using TVECTOR_INFO = TDynamicArrayTypeInfo<TELEMENT, TVECTOR>;

		static TypeInfo* Get() {
			static TVECTOR_INFO InstancedTypeInfo{
				"std::vector",
				sizeof( TVECTOR ),
				[]( TypeInfo* Info ) {
					Info->Description = "dynamic array";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( DynamicArrayTypeInfo* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};

	template<typename TELEMENT>
	struct TypeResolver<std::forward_list<TELEMENT>> {
		using TLIST = std::forward_list<TELEMENT>;
		using TLIST_INFO = TDynamicArrayTypeInfo<TELEMENT, TLIST>;

		static TypeInfo* Get() {
			static TLIST_INFO InstancedTypeInfo{
				"std::forward_list",
				sizeof( TLIST ),
				[]( TypeInfo* Info ) {
					Info->Description = "singly-linked list";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( DynamicArrayTypeInfo* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};

	template<typename TELEMENT>
	struct TypeResolver<std::list<TELEMENT>> {
		using TLIST = std::list<TELEMENT>;
		using TLIST_INFO = TDynamicArrayTypeInfo<TELEMENT, TLIST>;

		static TypeInfo* Get() {
			static TLIST_INFO InstancedTypeInfo{
				"std::list",
				sizeof( TLIST ),
				[]( TypeInfo* Info ) {
					Info->Description = "doubly-linked list";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( DynamicArrayTypeInfo* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};

	template<typename TELEMENT>
	struct TypeResolver<std::deque<TELEMENT>> {
		using TDEQUE = std::deque<TELEMENT>;
		using TDEQUE_INFO = TDynamicArrayTypeInfo<TELEMENT, TDEQUE>;

		static TypeInfo* Get() {
			static TDEQUE_INFO InstancedTypeInfo{
				"std::deque",
				sizeof( TDEQUE ),
				[]( TypeInfo* Info ) {
					Info->Description = "double-ended queue";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( auto* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};
}
