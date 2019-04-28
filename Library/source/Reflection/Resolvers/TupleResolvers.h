#include <tuple>
#include <numeric>
#include "Engine/Hash.h"
#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	namespace Internal {
		//============================================================
		// Standard tuple type specializations

		template<typename ...TELEMENTS>
		struct TypeResolver_Implementation<std::tuple<TELEMENTS...>> {
			using TTUPLE = std::tuple<TELEMENTS...>;
			static TTupleTypeInfo<TTUPLE, TELEMENTS...> const _TypeInfo;
			static TypeInfo const* Get() { return &_TypeInfo; }
			static constexpr Hash128 GetID() {
				constexpr size_t SIZE = std::tuple_size<TTUPLE>::value;
				Hash128 const IDs[SIZE] = { TypeResolver<TELEMENTS>::GetID()... };
				return std::accumulate( IDs + 1, IDs + SIZE, IDs[0] );
			}
		};
		template<typename ...TELEMENTS>
		TTupleTypeInfo<std::tuple<TELEMENTS...>, TELEMENTS...> const TypeResolver_Implementation<std::tuple<TELEMENTS...>>::_TypeInfo{ "tuple", nullptr };
	}
}
