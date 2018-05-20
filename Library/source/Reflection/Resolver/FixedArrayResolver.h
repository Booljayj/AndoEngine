#pragma once
#include <vector>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/FixedArrayTypeInfo.h"
#include "Reflection/TypeUtility.h"

namespace Reflection {
	//============================================================
	// Standard array type specializations

	template<typename TELEMENT, size_t SIZE>
	struct TypeResolver<std::array<TELEMENT, SIZE>> {
		static TFixedArrayTypeInfo<TELEMENT, std::array<TELEMENT, SIZE>> const InstancedTypeInfo;
		static TypeInfo const* Get() { return &InstancedTypeInfo; }
	};
	template<typename TELEMENT, size_t SIZE>
	TFixedArrayTypeInfo<TELEMENT, std::array<TELEMENT, SIZE>> const TypeResolver<std::array<TELEMENT, SIZE>>::InstancedTypeInfo{
		[]( FixedArrayTypeInfo* FixedArrayInfo ) {
			FixedArrayInfo->Description = "fixed array";
			FixedArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
		},
		"std::array", sizeof( std::array<TELEMENT, SIZE> )
	};
}
