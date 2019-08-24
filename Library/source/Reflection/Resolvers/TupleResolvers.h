#pragma once
#include <tuple>
#include <numeric>
#include "Engine/Hash.h"
#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard tuple type specializations

		template<typename ...ElementTypes>
		struct TypeResolver_Implementation<std::tuple<ElementTypes...>> {
			using TupleType = std::tuple<ElementTypes...>;
			static TTupleTypeInfo<TupleType, ElementTypes...> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() {
				constexpr size_t Size = std::tuple_size<TupleType>::value;
				Hash128 const IDs[Size] = { TypeResolver<ElementTypes>::GetID()... };
				return std::accumulate(IDs, IDs + Size, Hash128{ "std::tuple" });
			}
		};
		template<typename ...ElementTypes>
		TTupleTypeInfo<std::tuple<ElementTypes...>, ElementTypes...> const TypeResolver_Implementation<std::tuple<ElementTypes...>>::_TypeInfo{ "tuple", FTypeFlags::None, nullptr };
	}
}
