#pragma once
#include "Engine/STL.h"
#include "Reflection/VariantTypeInfo.h"

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard variant type specializations

		template<typename BaseType>
		struct TypeResolver_Implementation<std::optional<BaseType>> {
			static TOptionalTypeInfo<std::optional<BaseType>, BaseType> const typeInfo;
			static TypeInfo const* Get() { return &typeInfo; }
			static constexpr Hash128 GetID() { return Hash128{ "std::optional" } + TypeResolver<BaseType>::GetID(); }
		};
		template<typename BaseType>
		TOptionalTypeInfo<std::optional<BaseType>, BaseType> const TypeResolver_Implementation<std::optional<BaseType>>::typeInfo = TOptionalTypeInfo<std::optional<BaseType>, BaseType>{}
			.Description("optional");
	}
}
