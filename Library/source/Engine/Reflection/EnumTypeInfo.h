#pragma once
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	/** TypeInfo for an enum, which is a type that can be set equal to one of several discrete named values */
	struct EnumTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		using TypeInfo::GetName;
	 	static constexpr ETypeClassification Classification = ETypeClassification::Enumeration;

		virtual ~EnumTypeInfo() = default;

		/** The underlying type of the values in the enumeration */
		TypeInfo const* underlying = nullptr;

		virtual void Serialize(YAML::Node& node, void const* instance) const final { node = GetName(GetIndexOfValue(instance)); }
		virtual void Deserialize(YAML::Node const& node, void* instance) const final { underlying->Copy(instance, GetValue(GetIndexOfName(node.as<std::string>()))); }

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

	namespace Concepts {
		template<typename T>
		concept ReflectedEnum = ReflectedType<T> and requires (T a) {
			{ ::Reflection::Reflect<T>::Get() } -> std::convertible_to<EnumTypeInfo const&>;
		};
	}

	//============================================================
	// Templates

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

		TYPEINFO_BUILDER_METHODS(EnumType)
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

		TYPEINFO_BUILDER_METHODS(UnderlyingType)
		decltype(auto) ElementView(std::span<EnumPairType> inElementView) { elementView = inElementView; return *this; }
	};
}

namespace YAML {
	template<Reflection::Concepts::ReflectedEnum Type>
	struct convert<Type> {
		static Node encode(const Type& instance) {
			Reflection::EnumTypeInfo const& type = Reflect<Type>::Get();

			return Node{ type.GetName(type.GetIndexOfValue(&instance)) };
		}

		static bool decode(const Node& node, Type& instance) {
			Reflection::EnumTypeInfo const& type = Reflect<Type>::Get();

			type.underlying->Copy(&instance, type.GetValue(type.GetIndexOfName(node.as<std::string>())));
			return true;
		}
	};
}