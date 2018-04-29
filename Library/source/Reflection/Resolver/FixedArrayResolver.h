#pragma once
#include <vector>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/FixedArrayTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard array type specializations

	template<typename TELEMENT, size_t SIZE>
	struct TypeResolver<std::array<TELEMENT, SIZE>> {
		using TSEQUENCE = std::array<TELEMENT, SIZE>;
		using TSEQUENCE_INFO = TFixedArrayTypeInfo<TELEMENT, TSEQUENCE>;

		static TypeInfo* Get() {
			static TSEQUENCE_INFO InstancedTypeInfo{
				"std::array",
				sizeof( std::array<TELEMENT, SIZE> ),
				[]( TypeInfo* Info ) {
					Info->Description = "fixed array";
					//Info->Serializer = make_unique<ArraySerializer>();

					if( auto* FixedArrayInfo = Info->As<FixedArrayTypeInfo>() ) {
						FixedArrayInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};
}
