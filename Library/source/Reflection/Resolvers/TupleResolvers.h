#pragma once
#include "Engine/STL.h"
#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard tuple type specializations

	template<typename... ElementTypes>
	struct TypeResolver<std::tuple<ElementTypes...>> {
		using TupleType = std::tuple<ElementTypes...>;
		static TTupleTypeInfo<TupleType, ElementTypes...> const typeInfo;
		static TupleTypeInfo const* Get() { return &typeInfo; }
		static constexpr Hash128 GetID() {
			constexpr size_t Size = std::tuple_size<TupleType>::value;
			Hash128 const ids[Size] = { TypeResolver<ElementTypes>::GetID()... };
			return std::accumulate(ids, ids + Size, Hash128{ "std::tuple" });
		}
	};
	template<typename ...ElementTypes>
	TTupleTypeInfo<std::tuple<ElementTypes...>, ElementTypes...> const TypeResolver<std::tuple<ElementTypes...>>::typeInfo = TTupleTypeInfo<std::tuple<ElementTypes...>, ElementTypes...>{}
		.Description("tuple");
}
