#pragma once
#include <vector>
#include "Engine/ArrayView.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	/**
	 * Flags are a type wherein several components of some underlying type can be combined
	 * together into an aggregate with the same underlying type. The components must have
	 * unique values and names, and several aggregates may be predefined if appropriate.
	 */
	struct FlagsTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Flags;

		FlagsTypeInfo() = delete;
		FlagsTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TypeInfo const* InUnderlying
		);
		virtual ~FlagsTypeInfo() = default;

		TypeInfo const* Underlying = nullptr;

		/** Get the number of components that the flags have */
		virtual size_t GetComponentCount() const = 0;
		/** Get the number of predefined aggregates that the flags have */
		virtual size_t GetAggregateCount() const = 0;

		/** Get the name of the component at the specified index */
		virtual std::string_view GetComponentName(size_t Index) const = 0;
		/** Get the value of the component at the specified index */
		virtual void const* GetComponentValue(size_t Index) const = 0;
		/** Get the name of the aggregate at the specified index */
		virtual std::string_view GetAggregateName(size_t Index) const = 0;
		/** Get the value of the aggregate at the specified index */
		virtual void const* GetAggregateValue(size_t Index) const = 0;

		/** Get the index of the component with the specified name */
		virtual size_t GetIndexOfComponentName(std::string_view Name) const = 0;
		/** Get the index of the component with the specified value */
		virtual size_t GetIndexOfComponentValue(void const* Value) const = 0;
		/** Get the index of the first aggregate with the specified name */
		virtual size_t GetIndexOfAggregateName(std::string_view Name) const = 0;
		/** Get the index of the first aggregate with the specified value */
		virtual size_t GetIndexOfAggregateValue(void const* Value) const = 0;

		/** Get the name of the special "empty" aggregate, which contains no components */
		virtual std::string_view GetEmptyName() const = 0;
		/** Get the value of the special "empty" aggregate, which contains no components */
		virtual void const* GetEmptyValue() const = 0;

		/** Add the component or aggregate value to the aggregate */
		virtual void Add(void* Aggregate, void const* Value) const = 0;
		/** Subtract the component or aggregate value from the aggregate */
		virtual void Subtract(void* Aggregate, void const* Value) const = 0;
		/** Returns true if the aggregate contains the component or aggregate value */
		virtual bool Contains(void const* Aggregate, void const* Value) const = 0;
		/** Returns true if the aggregate is empty (contains no components) */
		virtual bool IsEmpty(void const* Aggregate) const = 0;

		/** Fills the output vector with the indices of any components in the aggregate */
		virtual void GetComponents(void const* Aggregate, std::vector<size_t>& ComponentIndices) const = 0;
	};

	//============================================================
	// Templates

	template<typename FlagsType>
	struct TStandardFlagsTypeInfo : public FlagsTypeInfo {
		using FlagsPairType = std::pair<std::string_view, FlagsType>;
		using UnderlyingType = typename std::underlying_type<FlagsType>::type;

		/** The components for the flags */
		TArrayView<FlagsPairType> ComponentsView;
		/** The aggregates for the flags */
		TArrayView<FlagsPairType> AggregatesView;

		/** Information about the flags "Empty" value */
		FlagsPairType const* Empty;

		TStandardFlagsTypeInfo(
			char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TArrayView<FlagsPairType> InComponentsView, TArrayView<FlagsPairType> InAggregatesView, FlagsPairType const* InEmpty)
		: FlagsTypeInfo(
			TypeResolver<FlagsType>::GetID(), GetCompilerDefinition<FlagsType>(),
			InDescription, InFlags, InSerializer,
			TypeResolver<UnderlyingType>::Get())
		, ComponentsView(InComponentsView)
		, AggregatesView(InAggregatesView)
		, Empty(InEmpty)
		{}

		STANDARD_TYPEINFO_METHODS(FlagsType)

		virtual size_t GetComponentCount() const final {return ComponentsView.size();};
		virtual size_t GetAggregateCount() const final {return AggregatesView.size();};

		virtual std::string_view GetComponentName(size_t Index) const final {
			if (Index < ComponentsView.size()) return ComponentsView[Index].first;
			return std::string_view{};
		}
		virtual void const* GetComponentValue(size_t Index) const final {
			if (Index < ComponentsView.size()) return &ComponentsView[Index].second;
			return nullptr;
		}
		virtual std::string_view GetAggregateName(size_t Index) const final {
			if (Index < AggregatesView.size()) return AggregatesView[Index].first;
			return std::string_view{};
		}
		virtual void const* GetAggregateValue(size_t Index) const final {
			if (Index < AggregatesView.size()) return &AggregatesView[Index].second;
			return nullptr;
		}

		virtual size_t GetIndexOfComponentName(std::string_view Name) const final {
			const auto Iter = std::find_if(ComponentsView.begin(), ComponentsView.end(), [&](const auto& Pair) {return Pair.first == Name;});
			return Iter - ComponentsView.begin();
		}
		virtual size_t GetIndexOfComponentValue(void const* Value) const final {
			const auto Iter = std::find_if(ComponentsView.begin(), ComponentsView.end(), [&](const auto& Pair) {return Pair.second == Cast(Value);});
			return Iter - ComponentsView.begin();
		}
		virtual size_t GetIndexOfAggregateName(std::string_view Name) const final {
			const auto Iter = std::find_if(AggregatesView.begin(), AggregatesView.end(), [&](const auto& Pair) {return Pair.first == Name;});
			return Iter - AggregatesView.begin();
		}
		virtual size_t GetIndexOfAggregateValue(void const* Value) const final {
			const auto Iter = std::find_if(AggregatesView.begin(), AggregatesView.end(), [&](const auto& Pair) {return Pair.second == Cast(Value);});
			return Iter - AggregatesView.begin();
		}

		virtual std::string_view GetEmptyName() const final {return Empty->first;}
		virtual void const* GetEmptyValue() const final {return &Empty->second;}

		virtual void Add(void* Aggregate, void const* Value) const final {Cast(Aggregate) |= Cast(Value);}
		virtual void Subtract(void* Aggregate, void const* Value) const final {Cast(Aggregate) &= ~Cast(Value);}
		virtual bool Contains(void const* Aggregate, void const* Value) const final {return (Cast(Aggregate) & Cast(Value)) == Cast(Value);}
		virtual bool IsEmpty(void const* Aggregate) const final {return Cast(Aggregate) == Empty->second;}

		virtual void GetComponents(void const* Aggregate, std::vector<size_t>& ComponentIndices) const final {
			const FlagsType AggregateValue = Cast(Aggregate);
			for (size_t Index = 0; Index < ComponentsView.size(); ++Index) {
				if ((ComponentsView[Index].second & AggregateValue) > 0) {
					ComponentIndices.push_back(Index);
				}
			}
		}
	};
}
