#pragma once
#include <tuple>

namespace TupleUtility {
	namespace detail {
		/** Runtime value visitor implementations */
		template<size_t TupleElementPosition>
		struct Visit_Impl {
			template<typename ReturnType, typename TupleType, typename VisitorType>
			inline static constexpr ReturnType Visit(TupleType const& Tuple, size_t DesiredIndex, VisitorType&& Visitor) {
				return DesiredIndex == (TupleElementPosition - 1U) ?
					Visitor(std::get<TupleElementPosition - 1U>(Tuple)) :
					Visit_Impl<TupleElementPosition - 1U>::template Visit<ReturnType>(Tuple, DesiredIndex, Visitor);
			}
			template<typename ReturnType, typename TupleType, typename VisitorType>
			inline static constexpr ReturnType Visit(TupleType& Tuple, size_t DesiredIndex, VisitorType&& Visitor) {
				return DesiredIndex == (TupleElementPosition - 1U) ?
					Visitor(std::get<TupleElementPosition - 1U>(Tuple)) :
					Visit_Impl<TupleElementPosition - 1U>::template Visit<ReturnType>(Tuple, DesiredIndex, Visitor);
			}
		};

		template<>
		struct Visit_Impl<0U> {
			template<typename ReturnType, typename TupleType, typename VisitorType>
			inline static constexpr ReturnType Visit(TupleType const&, size_t, VisitorType&&) noexcept(noexcept(ReturnType{})) {
				static_assert(std::is_default_constructible<ReturnType>::value, "Explicit return type of Visit method must be default-constructible");
				return ReturnType{};
			}
			template<typename ReturnType, typename TupleType, typename VisitorType>
			inline static constexpr ReturnType Visit(TupleType&, size_t, VisitorType&&) noexcept(noexcept(ReturnType{})) {
				static_assert(std::is_default_constructible<ReturnType>::value, "Explicit return type of Visit method must be default-constructible");
				return ReturnType{};
			}
		};

		/** Runtime type visitor implementations */
		template<size_t TupleElementPosition>
		struct VisitType_Impl {
			template<typename ReturnType, typename TupleType, template<typename> class VisitorTemplate>
			inline static constexpr ReturnType VisitType(size_t DesiredIndex) {
				return DesiredIndex == (TupleElementPosition - 1U) ?
					VisitorTemplate<typename std::tuple_element<TupleElementPosition - 1U, TupleType>::type>::Get() :
					VisitType_Impl<TupleElementPosition - 1U>::template VisitType<ReturnType, TupleType, VisitorTemplate>(DesiredIndex);
			}
		};

		template<>
		struct VisitType_Impl<0U> {
			template<typename ReturnType, typename TupleType, template<typename> class VisitorTemplate>
			inline static constexpr ReturnType VisitType(size_t DesiredIndex) {
				static_assert(std::is_default_constructible<ReturnType>::value, "Explicit return type of VisitType method must be default-constructible");
				return ReturnType{};
			}
		};
	}

	/** Visit the element at the specified index and call the visitor functor with that element */
	template<typename ReturnType, typename TupleType, typename VisitorType>
	inline constexpr ReturnType VisitAt(TupleType const& Tuple, size_t DesiredIndex, VisitorType Visitor) {
		return detail::Visit_Impl<std::tuple_size<TupleType>::value>::template Visit<ReturnType>(Tuple, DesiredIndex, Visitor);
	}
	template<typename ReturnType, typename TupleType, typename VisitorType>
	inline constexpr ReturnType VisitAt(TupleType& Tuple, size_t DesiredIndex, VisitorType Visitor) {
		return detail::Visit_Impl<std::tuple_size<TupleType>::value>::template Visit<ReturnType>(Tuple, DesiredIndex, Visitor);
	}

	/** Visit the type at the specified index, instantiate the template with that type, and invoke the templates "Get" method */
	template<typename ReturnType, typename TupleType, template<typename> class VisitorTemplate>
	inline constexpr ReturnType VisitTypeAt(size_t DesiredIndex) {
		return detail::VisitType_Impl<std::tuple_size<TupleType>::value>::template VisitType<ReturnType, TupleType, VisitorTemplate>(DesiredIndex);
	}
}
