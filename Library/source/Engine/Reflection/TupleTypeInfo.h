#pragma once
#include "Engine/Reflection/TypeInfo.h"
#include "Engine/TupleUtility.h"

namespace Reflection {
	/** TypeInfo for a tuple type. Tuples are an indexed collection of other types. */
	struct TupleTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Tuple;

		/** The types of each element in the tuple */
		std::vector<TypeInfo const*> types;

		virtual ~TupleTypeInfo() = default;

		/** Get the value at a specific index in the tuple */
		virtual void* GetValue(void* instance, size_t index) const = 0;
		virtual void const* GetValue(void const* instance, size_t index) const = 0;
	};

	//============================================================
	// Templates

	template<typename TupleType, typename... ElementTypes>
	struct TTupleTypeInfo : public ImplementedTypeInfo<TupleType, TupleTypeInfo> {
		using ImplementedTypeInfo<TupleType, TupleTypeInfo>::Cast;
		using TupleTypeInfo::types;

		TTupleTypeInfo(std::string_view name) : ImplementedTypeInfo<TupleType, TupleTypeInfo>(Reflect<TupleType>::ID, name) {
			types = { Reflect<ElementTypes>::Get() ... };
		}

		virtual void* GetValue(void* instance, size_t index) const final {
			return TupleUtility::VisitAt<void*>(Cast(instance), index, TupleUtility::PointerVisitor{});
		}
		virtual void const* GetValue(void const* instance, size_t index) const final {
			return TupleUtility::VisitAt<void const*>(Cast(instance), index, TupleUtility::PointerVisitor{});
		}
	};

	//============================================================
	// Standard tuple reflection

	template<typename... ElementTypes>
	struct Reflect<std::tuple<ElementTypes...>> {
		static TupleTypeInfo const& Get() { return info; }
		static constexpr Hash128 ID = Hash128{ "std::tuple"sv } + (... + Reflect<ElementTypes>::ID);
	private:
		using ThisTypeInfo = TTupleTypeInfo<std::tuple<ElementTypes...>, ElementTypes...>;
		static ThisTypeInfo const info;
	};
	template<typename ...ElementTypes>
	typename Reflect<std::tuple<ElementTypes...>>::ThisTypeInfo const Reflect<std::tuple<ElementTypes...>>::info =\
		Reflect<std::tuple<ElementTypes...>>::ThisTypeInfo{ "std::tuple"sv }
		.Description("tuple");
}
