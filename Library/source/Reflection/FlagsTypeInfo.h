#pragma once
#include <vector>
#include "Engine/ArrayView.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	//@todo Support aggregates and components with different types (such as TFlags)
	/**
	 * Flags are a type wherein several components of some underlying type can be combined
	 * together into an aggregate with the same underlying type. The components must have
	 * unique values and names, and several aggregates may be predefined if appropriate.
	 */
	struct FlagsTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Flags;

		FlagsTypeInfo() = delete;
		FlagsTypeInfo(Hash128 inID, CompilerDefinition inDef);
		virtual ~FlagsTypeInfo() = default;

		TypeInfo const* underlyingType = nullptr;

		/** Get the number of components that the flags have */
		virtual size_t GetComponentCount() const = 0;
		/** Get the number of predefined aggregates that the flags have */
		virtual size_t GetAggregateCount() const = 0;

		/** Get the name of the component at the specified index */
		virtual std::string_view GetComponentName(size_t index) const = 0;
		/** Get the value of the component at the specified index */
		virtual void const* GetComponentValue(size_t index) const = 0;
		/** Get the name of the aggregate at the specified index */
		virtual std::string_view GetAggregateName(size_t index) const = 0;
		/** Get the value of the aggregate at the specified index */
		virtual void const* GetAggregateValue(size_t index) const = 0;

		/** Get the index of the component with the specified name */
		virtual size_t GetIndexOfComponentName(std::string_view name) const = 0;
		/** Get the index of the component with the specified value */
		virtual size_t GetIndexOfComponentValue(void const* value) const = 0;
		/** Get the index of the first aggregate with the specified name */
		virtual size_t GetIndexOfAggregateName(std::string_view name) const = 0;
		/** Get the index of the first aggregate with the specified value */
		virtual size_t GetIndexOfAggregateValue(void const* value) const = 0;

		/** Get the name of the special "empty" aggregate, which contains no components */
		virtual std::string_view GetEmptyName() const = 0;
		/** Get the value of the special "empty" aggregate, which contains no components */
		virtual void const* GetEmptyValue() const = 0;

		/** Add the component or aggregate value to the aggregate */
		virtual void Add(void* aggregate, void const* value) const = 0;
		/** Subtract the component or aggregate value from the aggregate */
		virtual void Subtract(void* aggregate, void const* value) const = 0;
		/** Returns true if the aggregate contains the component or aggregate value */
		virtual bool Contains(void const* aggregate, void const* value) const = 0;
		/** Returns true if the aggregate is empty (contains no components) */
		virtual bool IsEmpty(void const* aggregate) const = 0;

		/** Fills the output vector with the indices of any components in the aggregate */
		virtual void GetComponents(void const* aggregate, std::vector<size_t>& componentIndices) const = 0;
	};

	//============================================================
	// Templates

	template<typename FlagsType>
	struct TStandardFlagsTypeInfo : public FlagsTypeInfo {
		using FlagsPairType = std::pair<std::string_view, FlagsType>;
		using UnderlyingType = typename std::underlying_type<FlagsType>::type;

		/** The components for the flags */
		TArrayView<FlagsPairType> componentsView;
		/** The aggregates for the flags */
		TArrayView<FlagsPairType> aggregatesView;

		/** Information about the flags "empty" value */
		FlagsPairType const* empty = nullptr;

		TStandardFlagsTypeInfo()
		: FlagsTypeInfo(TypeResolver<FlagsType>::GetID(), GetCompilerDefinition<FlagsType>())
		{
			underlyingType = TypeResolver<UnderlyingType>::Get();
		}

		STANDARD_TYPEINFO_METHODS(FlagsType)

		virtual size_t GetComponentCount() const final { return componentsView.size(); };
		virtual size_t GetAggregateCount() const final { return aggregatesView.size(); };

		virtual std::string_view GetComponentName(size_t index) const final {
			if (index < componentsView.size()) return componentsView[index].first;
			return std::string_view{};
		}
		virtual void const* GetComponentValue(size_t index) const final {
			if (index < componentsView.size()) return &componentsView[index].second;
			return nullptr;
		}
		virtual std::string_view GetAggregateName(size_t index) const final {
			if (index < aggregatesView.size()) return aggregatesView[index].first;
			return std::string_view{};
		}
		virtual void const* GetAggregateValue(size_t index) const final {
			if (index < aggregatesView.size()) return &aggregatesView[index].second;
			return nullptr;
		}

		virtual size_t GetIndexOfComponentName(std::string_view name) const final {
			const auto iter = std::find_if(componentsView.begin(), componentsView.end(), [&](const auto& Pair) { return Pair.first == name; });
			return iter - componentsView.begin();
		}
		virtual size_t GetIndexOfComponentValue(void const* value) const final {
			const auto iter = std::find_if(componentsView.begin(), componentsView.end(), [&](const auto& Pair) { return Pair.second == Cast(value); });
			return iter - componentsView.begin();
		}
		virtual size_t GetIndexOfAggregateName(std::string_view name) const final {
			const auto iter = std::find_if(aggregatesView.begin(), aggregatesView.end(), [&](const auto& Pair) { return Pair.first == name; });
			return iter - aggregatesView.begin();
		}
		virtual size_t GetIndexOfAggregateValue(void const* value) const final {
			const auto iter = std::find_if(aggregatesView.begin(), aggregatesView.end(), [&](const auto& Pair) { return Pair.second == Cast(value); });
			return iter - aggregatesView.begin();
		}

		virtual std::string_view GetEmptyName() const final { return empty->first; }
		virtual void const* GetEmptyValue() const final { return &empty->second; }

		virtual void Add(void* aggregate, void const* value) const final { Cast(aggregate) |= Cast(value); }
		virtual void Subtract(void* aggregate, void const* value) const final { Cast(aggregate) &= ~Cast(value); }
		virtual bool Contains(void const* aggregate, void const* value) const final { return (Cast(aggregate) & Cast(value)) == Cast(value); }
		virtual bool IsEmpty(void const* aggregate) const final { return Cast(aggregate) == empty->second; }

		virtual void GetComponents(void const* aggregate, std::vector<size_t>& componentIndices) const final {
			const FlagsType aggregateValue = Cast(aggregate);
			for (size_t index = 0; index < componentsView.size(); ++index) {
				if ((componentsView[index].second & aggregateValue) > 0) {
					componentIndices.push_back(index);
				}
			}
		}

		TStandardFlagsTypeInfo& ComponentsView(TArrayView<FlagsPairType> inComponentsView) { componentsView = inComponentsView; return *this; }
		TStandardFlagsTypeInfo& AggregatesView(TArrayView<FlagsPairType> inAggregatesView) { aggregatesView = inAggregatesView; return *this; }
		TStandardFlagsTypeInfo& Empty(FlagsPairType const* inEmpty) { empty = inEmpty; return *this; }
	};
}
