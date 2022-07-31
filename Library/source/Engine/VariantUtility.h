#pragma once
#include "Engine/STL.h"

namespace VariantUtility {
	/** Instantiate the template with each type in the variant, then call the ForEachMethod with the result of the template "Get" method and the index of the type */
	template<typename VariantType, template<typename> class VisitorTemplate, typename ForEachMethod, size_t ElementIndex = 0>
	inline constexpr void ForEachType(ForEachMethod&& method) {
		if constexpr (ElementIndex < std::variant_size_v<VariantType>) {
			using ElementType = std::variant_alternative_t<ElementIndex, VariantType>;
			if (method(ElementIndex, VisitorTemplate<ElementType>::Get())) {
				ForEachType<VariantType, VisitorTemplate, ForEachMethod, ElementIndex + 1>(std::forward<ForEachMethod>(method));
			}
		}
	}

	/** Instantiate the template with each type in the variant, then return the index of the type for which the result of the "Get" method matches the input value */
	template<typename VariantType, template<typename> class VisitorTemplate, typename ValueType, size_t ElementIndex = 0>
	inline constexpr size_t IndexOfType(ValueType value) {
		if constexpr (ElementIndex < std::variant_size_v<VariantType>) {
			using ElementType = std::variant_alternative_t<ElementIndex, VariantType>;
			if (VisitorTemplate<ElementType>::Get() == value) {
				return ElementIndex;
			} else {
				return IndexOfType<VariantType, VisitorTemplate, ValueType, ElementIndex + 1>(value);
			}
		} else {
			return std::variant_size_v<VariantType>;
		}
	}

	/** Set the value of the variant using a runtime index */
	template<typename VariantType, size_t ElementIndex = 0>
	inline constexpr void EmplaceIndex(VariantType& variant, size_t index) {
		if constexpr (ElementIndex < std::variant_size_v<VariantType>) {
			if (index == ElementIndex) {
				variant.template emplace<ElementIndex>();
			} else {
				EmplaceIndex<VariantType, ElementIndex + 1>(variant, index);
			}
		}
	}

	/** Set the value of the variant using a runtime index and a pointer to a value to copy */
	template<typename VariantType, size_t ElementIndex = 0>
	inline constexpr void EmplaceIndex(VariantType& variant, size_t index, void const* source) {
		if constexpr (ElementIndex < std::variant_size_v<VariantType>) {
			if (index == ElementIndex) {
				variant.template emplace<ElementIndex>(*static_cast<std::variant_alternative_t<ElementIndex, VariantType> const*>(source));
			} else {
				EmplaceIndex<VariantType, ElementIndex + 1>(variant, index, source);
			}
		}
	}

	/** Variant visitor which returns a type-erased pointer to the held value */
	struct PointerVisitor {
		template<typename T>
		void* operator()(T& value) const { return static_cast<void*>(&value); }
		template<typename T>
		void const* operator()(T const& value) const { return static_cast<void const*>(&value); }
	};
}
