#pragma once
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	template<typename EnumType>
	struct TStandardEnumTypeInfo : public ImplementedTypeInfo<EnumType, EnumTypeInfo> {
		using EnumPairType = std::pair<std::string_view, EnumType>;
		using UnderlyingType = typename std::underlying_type<EnumType>::type;
		using ImplementedTypeInfo<EnumType, EnumTypeInfo>::Cast;
		using EnumTypeInfo::underlying;

		/** The elements in this enumeration */
		std::span<EnumPairType> elementView;

		TStandardEnumTypeInfo(std::string_view inName) : ImplementedTypeInfo<EnumType, EnumTypeInfo>(Reflect<EnumType>::ID, inName) {
			underlying = Reflect<UnderlyingType>::Get();
		}

		virtual size_t GetCount() const final { elementView.size(); }

		virtual void const* GetValue(size_t index) const final {
			if (index < elementView.size()) return &elementView[index].second;
			return nullptr;
		}
		virtual std::string_view GetName(size_t index) const final {
			if (index < elementView.size()) return elementView[index].first;
			return std::string_view{};
		}

		virtual size_t GetIndexOfValue(void const* value) const final {
			const auto iter = std::find(elementView.begin(), elementView.end(), [&](auto const& pair) { return pair.second == Cast(value); });
			return iter - elementView.begin();
		}
		virtual size_t GetIndexOfName(std::string_view name) const final {
			const auto iter = std::find(elementView.begin(), elementView.end(), [&](auto const& pair) { return pair.first == name; });
			return iter - elementView.begin();
		}

		decltype(auto) ElementView(std::span<EnumPairType> inElementView) { elementView = inElementView; return *this; }
	};

	/** Generic type info that allows any array of name-value pairs to be considered an enum. Must be explicitly named, because there is no actual type involved. */
	template<typename UnderlyingType>
	struct TGenericEnumTypeInfo : public ImplementedTypeInfo<UnderlyingType, EnumTypeInfo> {
		using EnumPairType = std::pair<std::string_view, UnderlyingType>;
		using ImplementedTypeInfo<UnderlyingType, EnumTypeInfo>::Cast;
		using EnumTypeInfo::underlying;

		/** The elements in this enumeration */
		std::span<EnumPairType> elementView;

		TGenericEnumTypeInfo(Hash128 inID, std::string_view inName) : ImplementedTypeInfo<UnderlyingType, EnumTypeInfo>(inID, inName) {
			underlying = Reflect<UnderlyingType>::Get();
		}

		virtual size_t GetCount() const final { elementView.size(); }

		virtual void const* GetValue(size_t index) const final {
			if (index < elementView.size()) return &elementView[index].second;
			return nullptr;
		}
		virtual std::string_view GetName(size_t index) const final {
			if (index < elementView.size()) return elementView[index].first;
			return std::string_view{};
		}

		virtual size_t GetIndexOfValue(void const* value) const final {
			const auto iter = std::find(elementView.begin(), elementView.end(), [&](auto const& pair) { return pair.second == Cast(value); });
			return iter - elementView.begin();
		}
		virtual size_t GetIndexOfName(std::string_view name) const final {
			const auto iter = std::find(elementView.begin(), elementView.end(), [&](auto const& pair) { return pair.first == name; });
			return iter - elementView.begin();
		}

		decltype(auto) ElementView(std::span<EnumPairType> inElementView) { elementView = inElementView; return *this; }
	};
}
