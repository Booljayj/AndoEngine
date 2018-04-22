#pragma once
#include <vector>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/SequenceTypeInfo.h"

namespace Reflection {

	//============================================================
	// Standard sequence type specializations

	template<typename TELEMENT>
	struct TypeResolver<std::vector<TELEMENT>> {
		using TSEQUENCE = std::vector<TELEMENT>;
		using TSEQUENCE_INFO = TSequenceTypeInfo<TELEMENT, TSEQUENCE>;

		static TypeInfo* Get() {
			static TSEQUENCE_INFO InstancedTypeInfo{
				[]( TypeInfo* Info ) {
					Info->Name = "std::vector<T>";
					Info->Description = "dynamic array";
					Info->Size = sizeof( std::vector<TELEMENT> );
					//Info->Serializer = make_unique<SequenceSerializer>();

					if( SequenceTypeInfo* SequenceInfo = Info->As<SequenceTypeInfo>() ) {
						SequenceInfo->ElementType = TypeResolver<TELEMENT>::Get();
					}
				}
			};
			return &InstancedTypeInfo;
		}
	};
}
