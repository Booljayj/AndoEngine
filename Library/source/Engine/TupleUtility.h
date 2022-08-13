#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/TypeTraits.h"

/** Returns the sizes of all types in the tuple summed together, rather than the size of the tuple as a whole. This allows padding to be detected. */
template<typename TupleType>
struct TupleElementSizes {
	static constexpr size_t Sum() { return 0; }
};
template<typename... Types>
struct TupleElementSizes<std::tuple<Types...>> {
	static constexpr size_t Sum() { return SumTypeSizes<Types...>(); }
};

namespace TupleUtility {
	/** Invoke the visitor with every element in the tuple */
	template<typename TupleType, typename VisitorType, size_t ElementIndex = 0>
	inline constexpr void Visit(TupleType& tuple, VisitorType&& visitor) {
		if constexpr (ElementIndex < std::tuple_size_v<TupleType>) {
			visitor(std::get<ElementIndex>(tuple));
			Visit<TupleType, VisitorType, ElementIndex + 1>(tuple, std::forward<VisitorType>(visitor));
		}
	}

	/** Visit the element at the specified index and call the visitor functor with that element */
	template<typename TupleType, typename VisitorType, typename ReturnType, size_t ElementIndex = 0>
	inline constexpr ReturnType VisitAt(TupleType const& tuple, size_t index, VisitorType visitor) {
		if constexpr (ElementIndex < std::tuple_size_v<TupleType>) {
			if (index == ElementIndex) return visitor(std::get<ElementIndex>(tuple));
			else return VisitAt<TupleType, VisitorType, ReturnType, ElementIndex + 1>(tuple, index, visitor);
		}
		return ReturnType{};
	}
	template<typename ReturnType, typename TupleType, typename VisitorType, size_t ElementIndex = 0>
	inline constexpr ReturnType VisitAt(TupleType& tuple, size_t index, VisitorType visitor) {
		if constexpr (ElementIndex < std::tuple_size_v<TupleType>) {
			if (index == ElementIndex) return visitor(std::get<ElementIndex>(tuple));
			else return VisitAt<TupleType, VisitorType, ReturnType, ElementIndex + 1>(tuple, index, visitor);
		}
		return ReturnType{};
	}

	/** Visit the type at the specified index, instantiate the template with that type, and invoke the templates "Get" method */
	template<typename TupleType, template<typename> class VisitorTemplate, typename ReturnType, size_t ElementIndex = 0>
	inline constexpr ReturnType VisitTypeAt(size_t index) {
		if constexpr (ElementIndex < std::tuple_size_v<TupleType>) {
			using ElementType = std::tuple_element_t<ElementIndex, TupleType>;
			if (index == ElementIndex) return VisitorTemplate<ElementType>::Get();
			return VisitTypeAt<TupleType, VisitorTemplate, ReturnType, ElementIndex + 1>(index);
		}
		return ReturnType{};
	}

	/** Tuple element visitor which returns a type-erased pointer to the element */
	struct PointerVisitor {
		template<typename T>
		void* operator()(T& element) const { return static_cast<void*>(&element); }
		template<typename T>
		void const* operator()(T const& element) const { return static_cast<void const*>(&element); }
	};
}
