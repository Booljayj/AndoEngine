#include <memory>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/PolyTypeInfo.h"

namespace Reflection {
	namespace Internal {
	//============================================================
	// Standard poly type specializations

		template<typename TBASE>
		struct TypeResolver_Implementation<std::unique_ptr<TBASE>> {
			static TUniquePtrTypeInfo<TBASE> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() { return Hash128{ "std::unique_ptr" } + TypeResolver<TBASE>::GetID(); }
		};
		template<typename TBASE>
		TUniquePtrTypeInfo<TBASE> const TypeResolver_Implementation<std::unique_ptr<TBASE>>::_TypeInfo{ "unique pointer", nullptr };
	}
}
