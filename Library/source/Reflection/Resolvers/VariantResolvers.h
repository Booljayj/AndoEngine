#include <tuple>
#include "Reflection/VariantTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard variant type specializations

#if STD_OPTIONAL_SUPPORT
	template<typename TBASE>
	struct TypeResolver<std::optional<TBASE>> {
		static TOptionalTypeInfo<TBASE> const _TypeInfo;
		static TypeInfo const* Get() { return &_TypeInfo; }
		static constexpr sid_t GetID() { return id_combine( id( "std::optional" ), TypeResolver<TBASE>::GetID() ); }
	};
	template<typename TBASE>
	TOptionalTypeInfo<TBASE> const TypeResolver<std::optional<TBASE>>::_TypeInfo{ "optional", nullptr };
#endif
}
