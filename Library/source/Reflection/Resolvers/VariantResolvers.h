#include <tuple>
#include "Engine/Hash.h"
#include "Reflection/VariantTypeInfo.h"

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard variant type specializations

#if STD_OPTIONAL_SUPPORT
		template<typename TBASE>
		struct TypeResolver_Implementation<std::optional<TBASE>> {
			static TOptionalTypeInfo<TBASE> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() { return Hash128{ "std::optional" } + TypeResolver<TBASE>::GetID(); }
		};
		template<typename TBASE>
		TOptionalTypeInfo<TBASE> const TypeResolver_Implementation<std::optional<TBASE>>::_TypeInfo{ "optional", nullptr };
#endif
	}
}
