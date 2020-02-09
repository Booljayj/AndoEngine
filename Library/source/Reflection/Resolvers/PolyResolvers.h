#pragma once
#include <memory>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"
#include "Reflection/PolyTypeInfo.h"

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard poly type specializations

		template<typename BaseType>
		struct TypeResolver_Implementation<std::unique_ptr<BaseType>> {
			static TUniquePtrTypeInfo<BaseType> const typeInfo;
			static TypeInfo const* Get() { return &typeInfo; }
			static constexpr Hash128 GetID() { return Hash128{ "std::unique_ptr" } + TypeResolver<BaseType>::GetID(); }
		};
		template<typename BaseType>
		TUniquePtrTypeInfo<BaseType> const TypeResolver_Implementation<std::unique_ptr<BaseType>>::typeInfo{ "unique pointer", FTypeFlags::None, nullptr };
	}
}
