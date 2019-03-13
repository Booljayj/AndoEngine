#include <memory>
#include "Reflection/TypeResolver.h"
#include "Reflection/PolyTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard poly type specializations

	template<typename TBASE>
	struct TypeResolver<std::unique_ptr<TBASE>> {
		static TUniquePtrTypeInfo<TBASE> const _TypeInfo;
		static TypeInfo const* Get() { return &_TypeInfo; }
		static constexpr sid_t GetID() { return id_combine( id( "std::unique_ptr" ), TypeResolver<TBASE>::GetID() ); }
	};
	template<typename TBASE>
	TUniquePtrTypeInfo<TBASE> const TypeResolver<std::unique_ptr<TBASE>>::_TypeInfo{ "unique pointer", nullptr };
}
