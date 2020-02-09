#pragma once
#include <tuple>
#include <numeric>
#include "Engine/Hash.h"
#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard tuple type specializations

		template<typename... ElementTypes>
		struct TypeResolver_Implementation<std::tuple<ElementTypes...>> {
			using TupleType = std::tuple<ElementTypes...>;
			static TTupleTypeInfo<TupleType, ElementTypes...> const typeInfo;
			static TypeInfo const* Get() { return &typeInfo; }
			static constexpr Hash128 GetID() {
				constexpr size_t Size = std::tuple_size<TupleType>::value;
				Hash128 const ids[Size] = { TypeResolver<ElementTypes>::GetID()... };
				return std::accumulate(ids, ids + Size, Hash128{ "std::tuple" });
			}
		};
		template<typename ...ElementTypes>
		TTupleTypeInfo<std::tuple<ElementTypes...>, ElementTypes...> const TypeResolver_Implementation<std::tuple<ElementTypes...>>::typeInfo{ "tuple", FTypeFlags::None, nullptr };
	}
}
