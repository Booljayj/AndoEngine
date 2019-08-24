#pragma once
#include "Engine/ArrayView.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct EnumerationTypeInfo : public TypeInfo {
	 	static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Enumeration;

		EnumerationTypeInfo() = delete;
		EnumerationTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TypeInfo const* InUnderlyingTypeInfo
		);
		virtual ~EnumerationTypeInfo() = default;

		/** The underlying type of the values in the enumeration */
		TypeInfo const* UnderlyingTypeInfo = nullptr;

		/** Get the number of values that the enumeration defines */
		virtual size_t GetCount() const = 0;

		/** Get the value at the specified index */
		virtual void const* GetValue(size_t Index) const = 0;
		/** Get the name of the value at the specified index */
		virtual std::string_view GetName(size_t Index) const = 0;

		/** Get the index of the first element with the specified value */
		virtual size_t GetIndexOfValue(void const* Value) const = 0;
		/** Get the index of the first element with the specified name */
		virtual size_t GetIndexOfName(std::string_view Name) const = 0;
	};

	//============================================================
	// Templates

	template<typename EnumType>
	struct TStandardEnumerationTypeInfo : public EnumerationTypeInfo {
		using EnumPairType = std::pair<std::string_view, EnumType>;
		using UnderlyingType = typename std::underlying_type<EnumType>::type;

		/** The elements in this enumeration */
		TArrayView<EnumPairType> ElementView;

		TStandardEnumerationTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TArrayView<EnumPairType> InElementView)
		: EnumerationTypeInfo(
			TypeResolver<EnumType>::GetID(), GetCompilerDefinition<EnumType>(),
			InDescription, InFlags, InSerializer,
			TypeResolver<UnderlyingType>::Get())
		, ElementView(InElementView)
		{}

		STANDARD_TYPEINFO_METHODS(EnumType)

		virtual size_t GetCount() const final { ElementView.size(); }

		virtual void const* GetValue(size_t Index) const final {
			if (Index < ElementView.size()) return &ElementView[Index].second;
			return nullptr;
		}
		virtual std::string_view GetName(size_t Index) const final {
			if (Index < ElementView.size()) return ElementView[Index].first;
			return std::string_view{};
		}

		virtual size_t GetIndexOfValue(void const* Value) const final {
			const auto Iter = std::find(ElementView.begin(), ElementView.end(), [&](auto const& Pair) { return Pair.second == Cast(Value); });
			return Iter - ElementView.begin();
		}
		virtual size_t GetIndexOfName(std::string_view Name) const final {
			const auto Iter = std::find(ElementView.begin(), ElementView.end(), [&](auto const& Pair) { return Pair.first == Name; });
			return Iter - ElementView.begin();
		}
	};

	/** Generic type info that allows any array of name-value pairs to be considered an enum */
	template<typename UnderlyingType>
	struct TGenericEnumerationTypeInfo : public EnumerationTypeInfo {
		using EnumPairType = std::pair<std::string_view, UnderlyingType>;

		/** The elements in this enumeration */
		TArrayView<EnumPairType> ElementView;

		TGenericEnumerationTypeInfo(
			Hash128 InUniqueID,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TArrayView<EnumPairType> InElementView)
		: EnumerationTypeInfo(
			InUniqueID, GetCompilerDefinition<UnderlyingType>(),
			InDescription, InFlags, InSerializer,
			TypeResolver<UnderlyingType>::Get())
		, ElementView(InElementView)
		{}

		STANDARD_TYPEINFO_METHODS(UnderlyingType)

		virtual size_t GetCount() const final { ElementView.size(); }

		virtual void const* GetValue(size_t Index) const final {
			if (Index < ElementView.size()) return &ElementView[Index].second;
			return nullptr;
		}
		virtual std::string_view GetName(size_t Index) const final {
			if (Index < ElementView.size()) return ElementView[Index].first;
			return std::string_view{};
		}

		virtual size_t GetIndexOfValue(void const* Value) const final {
			const auto Iter = std::find(ElementView.begin(), ElementView.end(), [&](auto const& Pair) { return Pair.second == Cast(Value); });
			return Iter - ElementView.begin();
		}
		virtual size_t GetIndexOfName(std::string_view Name) const final {
			const auto Iter = std::find(ElementView.begin(), ElementView.end(), [&](auto const& Pair) { return Pair.first == Name; });
			return Iter - ElementView.begin();
		}
	};
}
