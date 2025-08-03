#pragma once
#include "Engine/Reflection/TypeInfo.h"
#include "Engine/FunctionRef.h"

//============================================================
// EXPERIMENTAL

namespace Reflection {
	template<typename FlagsType>
	struct TStandardFlagsTypeInfo : public ImplementedTypeInfo<FlagsType, FlagsTypeInfo> {
		using FlagsPairType = std::pair<std::string_view, FlagsType>;
		using UnderlyingType = typename std::underlying_type<FlagsType>::type;
		using ImplementedTypeInfo<FlagsType, FlagsTypeInfo>::Cast;
		using FlagsTypeInfo::underlying;

		/** The components for the flags */
		std::span<FlagsPairType> componentsView;
		/** The aggregates for the flags */
		std::span<FlagsPairType> aggregatesView;

		/** Information about the flags "empty" value */
		FlagsPairType const* empty = nullptr;

		TStandardFlagsTypeInfo(std::string_view inName) : ImplementedTypeInfo<FlagsType, FlagsTypeInfo>(Reflect<FlagsType>::ID, inName) {
			underlying = Reflect<UnderlyingType>::Get();
		}

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

		virtual void Reset(void* aggregate) const final { Cast(aggregate) = static_cast<FlagsType>(0); }
		virtual void Add(void* aggregate, void const* value) const final { Cast(aggregate) |= Cast(value); }
		virtual void Subtract(void* aggregate, void const* value) const final { Cast(aggregate) &= ~Cast(value); }
		virtual bool Contains(void const* aggregate, void const* value) const final { return (Cast(aggregate) & Cast(value)) == Cast(value); }
		virtual bool IsEmpty(void const* aggregate) const final { return Cast(aggregate) == empty->second; }

		virtual void ForEachComponent(void const* aggregate, FunctionRef<bool(size_t)> functor) const override final {
			const FlagsType aggregateValue = Cast(aggregate);
			for (size_t index = 0; index < componentsView.size(); ++index) {
				if ((componentsView[index].second & aggregateValue) > 0) {
					functor(index);
				}
			}
		}

		TStandardFlagsTypeInfo& ComponentsView(std::span<FlagsPairType> inComponentsView) { componentsView = inComponentsView; return *this; }
		TStandardFlagsTypeInfo& AggregatesView(std::span<FlagsPairType> inAggregatesView) { aggregatesView = inAggregatesView; return *this; }
		TStandardFlagsTypeInfo& Empty(FlagsPairType const* inEmpty) { empty = inEmpty; return *this; }
	};
}
