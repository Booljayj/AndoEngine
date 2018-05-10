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
		using TSEQUENCE = std::array<TELEMENT, SIZE>;
		using TSEQUENCE_INFO = TFixedArrayTypeInfo<TELEMENT, TSEQUENCE>;

		static TypeInfo* Get() {
			static TSEQUENCE_INFO InstancedTypeInfo{
				[]( TypeInfo* Info ) {
					Info->Description = "fixed array";
					//Info->Serializer = make_unique<ArraySerializer>();

					if( auto* FixedArrayInfo = Info->As<FixedArrayTypeInfo>() ) {
						FixedArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				},
				MakeArrayName( TypeResolver<TELEMENT>::Get(), SIZE ), sizeof( std::array<TELEMENT, SIZE> )
			};
			return &InstancedTypeInfo;
		}
	};
}
