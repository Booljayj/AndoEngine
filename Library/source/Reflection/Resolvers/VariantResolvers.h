#pragma once
#include <tuple>
#include "Engine/Hash.h"
#include "Reflection/VariantTypeInfo.h"

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard variant type specializations

#if STD_OPTIONAL_SUPPORT
		template<typename BaseType>
		struct TypeResolver_Implementation<std::optional<BaseType>> {
			static TOptionalTypeInfo<BaseType> const typeInfo;
			static TypeInfo const* Get() { return &typeInfo; }
			static constexpr Hash128 GetID() { return Hash128{ "std::optional" } + TypeResolver<BaseType>::GetID(); }
		};
		template<typename BaseType>
		TOptionalTypeInfo<BaseType> const TypeResolver_Implementation<std::optional<BaseType>>::typeInfo{ "optional", FTypeFlags::None, nullptr };
#endif
	}
}
