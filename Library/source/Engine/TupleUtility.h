#pragma once
#include <tuple>

namespace TupleUtility {
	namespace detail {
		/** Runtime value visitor implementations */
		template<size_t I>
		struct Visit_Impl {
			template<typename R, typename TTUPLE, typename TVISITOR>
			inline static constexpr R Visit( TTUPLE const& Tuple, size_t Index, TVISITOR Visitor ) {
				return Index == (I - 1U) ?
					Visitor( std::get<I - 1U>(Tuple) ) :
					Visit_Impl<I - 1U>::template Visit<R>( Tuple, Index, Visitor );
			}
			template<typename R, typename TTUPLE, typename TVISITOR>
			inline static constexpr R Visit( TTUPLE& Tuple, size_t Index, TVISITOR Visitor ) {
				return Index == (I - 1U) ?
					Visitor( std::get<I - 1U>(Tuple) ) :
					Visit_Impl<I - 1U>::template Visit<R>( Tuple, Index, Visitor );
			}
		};

		template<>
		struct Visit_Impl<0U> {
			template<typename R, typename TTUPLE, typename TVISITOR>
			inline static constexpr R Visit( TTUPLE const&, size_t, TVISITOR ) noexcept(noexcept(R{})) {
				static_assert( std::is_default_constructible<R>::value, "Explicit return type of Visit method must be default-constructible" );
				return R{};
			}
			template<typename R, typename TTUPLE, typename TVISITOR>
			inline static constexpr R Visit( TTUPLE&, size_t, TVISITOR ) noexcept(noexcept(R{})) {
				static_assert( std::is_default_constructible<R>::value, "Explicit return type of Visit method must be default-constructible" );
				return R{};
			}
		};

		/** Runtime type visitor implementations */
		template<size_t I>
		struct VisitType_Impl {
			template<typename R, typename TTUPLE, template<typename> class TVISITOR>
			inline static constexpr R VisitType( size_t Index ) {
				return Index == (I - 1U) ?
					TVISITOR<typename std::tuple_element<I - 1U, TTUPLE>::type>::Get() :
					VisitType_Impl<I - 1U>::template VisitType<R, TTUPLE, TVISITOR>( Index );
			}
		};

		template<>
		struct VisitType_Impl<0U> {
			template<typename R, typename TTUPLE, template<typename> class TVISITOR>
			inline static constexpr R VisitType( size_t Index ) {
				static_assert( std::is_default_constructible<R>::value, "Explicit return type of VisitType method must be default-constructible" );
				return R{};
			}
		};
	}

	/** Visit the element at the specified index and call the visitor functor with that element */
	template<typename R, typename TTUPLE, typename TVISITOR>
	inline constexpr R VisitAt( TTUPLE const& Tuple, size_t Index, TVISITOR Visitor ) {
		return detail::Visit_Impl<std::tuple_size<TTUPLE>::value>::template Visit<R>( Tuple, Index, Visitor );
	}
	template<typename R, typename TTUPLE, typename TVISITOR>
	inline constexpr R VisitAt( TTUPLE& Tuple, size_t Index, TVISITOR Visitor ) {
		return detail::Visit_Impl<std::tuple_size<TTUPLE>::value>::template Visit<R>( Tuple, Index, Visitor );
	}

	/** Visit the type at the specified index, instantiate the template with that type, and invoke the templates "Get" method */
	template<typename R, typename TTUPLE, template<typename> class TVISITOR>
	inline constexpr R VisitTypeAt( size_t Index ) {
		return detail::VisitType_Impl<std::tuple_size<TTUPLE>::value>::template VisitType<R, TTUPLE, TVISITOR>( Index );
	}
}
