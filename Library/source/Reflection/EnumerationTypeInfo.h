#pragma once
#include "Engine/ArrayView.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct EnumerationTypeInfo : public TypeInfo {
	 	static constexpr ETypeClassification Classification = ETypeClassification::Enumeration;

		EnumerationTypeInfo() = delete;
		EnumerationTypeInfo(Hash128 inID, CompilerDefinition inDef);
		virtual ~EnumerationTypeInfo() = default;

		/** The underlying type of the values in the enumeration */
		TypeInfo const* underlyingType = nullptr;

		/** Get the number of values that the enumeration defines */
		virtual size_t GetCount() const = 0;

		/** Get the value at the specified index */
		virtual void const* GetValue(size_t index) const = 0;
		/** Get the name of the value at the specified index */
		virtual std::string_view GetName(size_t index) const = 0;

		/** Get the index of the first element with the specified value */
		virtual size_t GetIndexOfValue(void const* value) const = 0;
		/** Get the index of the first element with the specified name */
		virtual size_t GetIndexOfName(std::string_view name) const = 0;
	};

	//============================================================
	// Templates

	template<typename EnumType>
	struct TStandardEnumerationTypeInfo : public EnumerationTypeInfo {
		using EnumPairType = std::pair<std::string_view, EnumType>;
		using UnderlyingType = typename std::underlying_type<EnumType>::type;

		/** The elements in this enumeration */
		TArrayView<EnumPairType> elementView;

		TStandardEnumerationTypeInfo()
		: EnumerationTypeInfo(TypeResolver<EnumType>::GetID(), GetCompilerDefinition<EnumType>())
		{
			underlyingType = TypeResolver<UnderlyingType>::Get();
		}

		STANDARD_TYPEINFO_METHODS(EnumType)

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

		TStandardEnumerationTypeInfo& ElementView(TArrayView<EnumPairType> inElementView) { elementView = inElementView; return *this; }
	};

	/** Generic type info that allows any array of name-value pairs to be considered an enum. Must be explicitly named, because there is no actual type involved. */
	template<typename UnderlyingType>
	struct TGenericEnumerationTypeInfo : public EnumerationTypeInfo {
		using EnumPairType = std::pair<std::string_view, UnderlyingType>;

		/** The elements in this enumeration */
		TArrayView<EnumPairType> elementView;

		TGenericEnumerationTypeInfo(Hash128 inID)
		: EnumerationTypeInfo(inID, GetCompilerDefinition<UnderlyingType>())
		{
			underlyingType = TypeResolver<UnderlyingType>::Get();
		}

		STANDARD_TYPEINFO_METHODS(UnderlyingType)

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

		TGenericEnumerationTypeInfo& ElementView(TArrayView<EnumPairType> inElementView) { elementView = inElementView; return *this; }
	};
}
