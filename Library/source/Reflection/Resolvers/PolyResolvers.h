#pragma once
#include "Engine/STL.h"
#include "Reflection/PolyTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard poly type specializations

	template<typename BaseType>
	struct TypeResolver<std::unique_ptr<BaseType>> {
		static TUniquePtrTypeInfo<BaseType> const typeInfo;
		static PolyTypeInfo const* Get() { return &typeInfo; }
		static constexpr Hash128 GetID() { return Hash128{ "std::unique_ptr" } + TypeResolver<BaseType>::GetID(); }
	};
	template<typename BaseType>
	TUniquePtrTypeInfo<BaseType> const TypeResolver<std::unique_ptr<BaseType>>::typeInfo = TUniquePtrTypeInfo<BaseType>{}
		.Description("unique pointer");
}
