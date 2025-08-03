#pragma once
#include <variant>
#include "Engine/Core.h"

//@todo Implement proper serialization for variants. Right now these are just stubs that don't serialize anything.

namespace Archive {
	template<typename... Types>
	struct Serializer<std::variant<Types...>> {
		static void Write(Output& archive, std::variant<Types...> const& variant) { }
		static void Read(Input& archive, std::variant<Types...>& variant) {}
	};
}

namespace Reflection {
	template<typename VariantType, typename... ValueTypes>
	struct TVariantTypeInfo : public ImplementedTypeInfo<VariantType, VariantTypeInfo> {
		using ImplementedTypeInfo<VariantType, VariantTypeInfo>::Cast;
		using VariantTypeInfo::types;

		std::vector<TypeInfo const*> types;

		TVariantTypeInfo(std::string_view inName) : ImplementedTypeInfo<VariantType, VariantTypeInfo>(Reflect<VariantType>::ID, inName) {
			types = { Reflect<ValueTypes>::Get() ... };
		}

		virtual std::span<TypeInfo const*> GetTypes() const override final { return types; }

		virtual TypeInfo const& GetType(void const* instance) const override final {
			return std::visit([](auto const& value) -> void const* { return Reflect<decltype(value)>::Get(); }, Cast(instance));
		}

		virtual void* GetValue(void* instance) const override final {
			return std::visit([](auto& value) -> void* { return static_cast<void*>(&value); }, Cast(instance));
		};
		virtual void const* GetValue(void const* instance) const override final {
			return std::visit([](auto const& value) -> void const* { return static_cast<void const*>(&value); }, Cast(instance));
		};

		virtual bool Assign(void* instance, TypeInfo const& type, void const* source) const override final {
			const auto iter = std::find(types.begin(), types.end(), &type);
			if (iter != types.end()) {
				const size_t index = (iter - types.begin());

				if (source) EmplaceIndex(Cast(instance), index, source);
				else EmplaceIndex(Cast(instance), index);
				return true;
			}
			return false;
		}

	private:
		/** Set the value of the variant using a runtime index */
		template<typename VariantType, size_t ElementIndex = 0>
		static constexpr void EmplaceIndex(VariantType& variant, size_t index) {
			if constexpr (ElementIndex < std::variant_size_v<VariantType>) {
				if (index == ElementIndex) {
					variant.template emplace<ElementIndex>();
				} else {
					EmplaceIndex<VariantType, ElementIndex + 1>(variant, index);
				}
			}
		}

		/** Set the value of the variant using a runtime index and a pointer to a value to copy */
		template<typename VariantType, size_t ElementIndex = 0>
		static constexpr void EmplaceIndex(VariantType& variant, size_t index, void const* source) {
			if constexpr (ElementIndex < std::variant_size_v<VariantType>) {
				if (index == ElementIndex) {
					variant.template emplace<ElementIndex>(*static_cast<std::variant_alternative_t<ElementIndex, VariantType> const*>(source));
				} else {
					EmplaceIndex<VariantType, ElementIndex + 1>(variant, index, source);
				}
			}
		}
	};
}

template<typename... Types>
struct Reflect<std::variant<Types...>> {
	static ::Reflection::VariantTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = (Hash128{ "std::variant"sv } + ... + Reflect<Types>::ID);
private:
	using ThisTypeInfo = ::Reflection::TVariantTypeInfo<std::variant<Types...>, Types...>;
	static ThisTypeInfo const info;
};
template<typename... Types>
typename Reflect<std::variant<Types...>>::ThisTypeInfo const Reflect<std::variant<Types...>>::info =
	Reflect<std::variant<Types...>>::ThisTypeInfo{ "std::variant"sv }
	.Description("variant");
