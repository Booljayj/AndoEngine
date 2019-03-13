#include <tuple>
#include "Reflection/TupleTypeInfo.h"

namespace Reflection {
	//============================================================
	// Standard tuple type specializations

	template<typename ...TELEMENTS>
	struct TypeResolver<std::tuple<TELEMENTS...>> {
		using TTUPLE = std::tuple<TELEMENTS...>;
		static TTupleTypeInfo<TTUPLE, TELEMENTS...> const _TypeInfo;
		static TypeInfo const* Get() { return &_TypeInfo; }
		static constexpr sid_t GetID() {
			constexpr size_t Size = std::tuple_size<TTUPLE>::value;
			const sid_t IDs[Size] = { TypeResolver<TELEMENTS>::GetID()... };
			return id_combine( IDs, Size );
		}
	};
	template<typename ...TELEMENTS>
	TTupleTypeInfo<std::tuple<TELEMENTS...>, TELEMENTS...> const TypeResolver<std::tuple<TELEMENTS...>>::_TypeInfo{ "tuple", nullptr };
}
