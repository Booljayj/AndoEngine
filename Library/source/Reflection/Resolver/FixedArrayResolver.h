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
		static_assert( SIZE < UINT32_MAX, "Fixed-size arrays larger than the capacity of a uint32 are not supported" );
		static TFixedArrayTypeInfo<TELEMENT, std::array<TELEMENT, SIZE>> const InstancedTypeInfo;
		static TypeInfo const* Get() { return &InstancedTypeInfo; }
		static std::string_view GetName() {
			static std::string const Name = MakeTemplateName<TELEMENT, std::integral_constant<size_t, SIZE>>( "std::array" );
			return Name;
		}
	};
	template<typename TELEMENT, size_t SIZE>
	TFixedArrayTypeInfo<TELEMENT, std::array<TELEMENT, SIZE>> const TypeResolver<std::array<TELEMENT, SIZE>>::InstancedTypeInfo{
		[]( FixedArrayTypeInfo* FixedArrayInfo ) {
			static_assert( SIZE < UINT32_MAX, "Fixed-size arrays larger than the capacity of a uint32 are not supported" );
			FixedArrayInfo->Description = "fixed array";
			FixedArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
		}
	};
}
