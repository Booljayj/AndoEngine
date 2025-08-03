#pragma once
#include <tuple>
#include "Engine/Core.h"
#include "Engine/TupleUtility.h"

namespace Reflection {
	template<typename TupleType, typename... ElementTypes>
	struct TTupleTypeInfo : public ImplementedTypeInfo<TupleType, TupleTypeInfo> {
		using ImplementedTypeInfo<TupleType, TupleTypeInfo>::Cast;
		using TupleTypeInfo::types;

		std::vector<TypeInfo const*> types;

		TTupleTypeInfo(std::u16string_view name, std::u16string_view description) : ImplementedTypeInfo<TupleType, TupleTypeInfo>(Reflect<TupleType>::ID, name, description) {
			types = { Reflect<ElementTypes>::Get() ... };
		}

		virtual std::span<TypeInfo const*> GetTypes() const override final { return types; }

		virtual void* GetValue(void* instance, size_t index) const override final {
			return TupleUtility::VisitAt<void*>(Cast(instance), index, TupleUtility::PointerVisitor{});
		}
		virtual void const* GetValue(void const* instance, size_t index) const override final {
			return TupleUtility::VisitAt<void const*>(Cast(instance), index, TupleUtility::PointerVisitor{});
		}
	};
}

template<typename... Types>
struct Reflect<std::tuple<Types...>> {
	static ::Reflection::TupleTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = (Hash128{ "std::tuple"sv } + ... + Reflect<Types>::ID);
private:
	using ThisTypeInfo = ::Reflection::TTupleTypeInfo<std::tuple<Types...>, Types...>;
	static ThisTypeInfo const info;
};
template<typename ...Types>
typename Reflect<std::tuple<Types...>>::ThisTypeInfo const Reflect<std::tuple<Types...>>::info =
	Reflect<std::tuple<Types...>>::ThisTypeInfo{ u"std::tuple"sv, u"tuple"sv };
