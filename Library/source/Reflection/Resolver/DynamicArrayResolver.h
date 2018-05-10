#pragma once
#include <string_view>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/DynamicArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	//============================================================
	// Standard dynamic array type specializations

	template<typename TELEMENT>
	struct TypeResolver<std::vector<TELEMENT>> {
		using TVECTOR = std::vector<TELEMENT>;
		using TVECTOR_INFO = TDynamicArrayTypeInfo<TELEMENT, TVECTOR>;

		static TypeInfo* Get() {
			static TVECTOR_INFO InstancedTypeInfo{
				[]( TypeInfo* Info ) {
					Info->Description = "dynamic array";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( DynamicArrayTypeInfo* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				},
				MakeTemplateName( "std::vector", { TypeResolver<TELEMENT>::Get() } ), sizeof( TVECTOR )
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
				[]( TypeInfo* Info ) {
					Info->Description = "singly-linked list";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( DynamicArrayTypeInfo* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				},
				MakeTemplateName( "std::forward_list", { TypeResolver<TELEMENT>::Get() } ), sizeof( TLIST )
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
				[]( TypeInfo* Info ) {
					Info->Description = "doubly-linked list";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( DynamicArrayTypeInfo* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				},
				MakeTemplateName( "std::list", { TypeResolver<TELEMENT>::Get() } ), sizeof( TLIST )
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
				[]( TypeInfo* Info ) {
					Info->Description = "double-ended queue";
					//Info->Serializer = make_unique<DynamicArraySerializer>();

					if( auto* DynamicArrayInfo = Info->As<DynamicArrayTypeInfo>() ) {
						DynamicArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				},
				MakeTemplateName( "std::deque", { TypeResolver<TELEMENT>::Get() } ), sizeof( TDEQUE )
			};
			return &InstancedTypeInfo;
		}
	};
}
