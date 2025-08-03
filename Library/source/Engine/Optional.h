#pragma once
#include <optional>
#include "Engine/Core.h"
#include "Engine/Reflection/PrimitiveTypeInfo.h"
#include "Engine/Reflection/VariantTypeInfo.h"

//@todo Implement proper serialization for optionals. Right now these are just stubs that don't serialize anything.

namespace Archive {
	template<typename T>
	struct Serializer<std::optional<T>> {
		static void Write(Output& archive, std::optional<T> const& optional) {}
		static void Read(Input& archive, std::optional<T>& optional) {}
	};
}

namespace Reflection {
	template<typename OptionalType, typename ValueType>
	struct TOptionalTypeInfo : public ImplementedTypeInfo<OptionalType, VariantTypeInfo> {
		using ImplementedTypeInfo<OptionalType, VariantTypeInfo>::Cast;
		using VariantTypeInfo::types;

		TOptionalTypeInfo(std::string_view inName) : ImplementedTypeInfo<OptionalType, VariantTypeInfo>(Reflect<OptionalType>::ID, inName) {
			types = { Reflect<void>::Get(), Reflect<ValueType>::Get() };
		}

		static constexpr ValueType const& CastValue(void const* value) { return *static_cast<ValueType const*>(value); };

		virtual bool CanContain(TypeInfo const* type) const final {
			return type == &Reflect<void>::Get() || type == &Reflect<ValueType>::Get();
		}

		virtual TypeInfo const& GetType(void const* instance) const {
			if (Cast(instance)) return Reflect<ValueType>::Get();
			else return Reflect<void>::Get();
		}

		virtual void* GetValue(void* instance) const final {
			OptionalType& opt = Cast(instance);
			if (opt) return &opt.value();
			else return nullptr;
		};
		virtual void const* GetValue(void const* instance) const final {
			OptionalType const& opt = Cast(instance);
			if (opt) return &opt.value();
			else return nullptr;
		};

		virtual bool Assign(void* instance, TypeInfo const& type, void const* source) const final {
			if (type == Reflect<ValueType>::Get()) {
				if (source) Cast(instance).emplace(CastValue(source));
				else Cast(instance).emplace();
				return true;
			} else if (type == Reflect<void>::Get()) {
				Cast(instance).reset();
				return true;
			} else {
				return false;
			}
		}
	};
}

template<typename BaseType>
struct Reflect<std::optional<BaseType>> {
	static ::Reflection::VariantTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::optional"sv } + Reflect<BaseType>::ID;
private:
	using ThisTypeInfo = ::Reflection::TOptionalTypeInfo<std::optional<BaseType>, BaseType>;
	static ThisTypeInfo const info;
};
template<typename BaseType>
typename Reflect<std::optional<BaseType>>::ThisTypeInfo const Reflect<std::optional<BaseType>>::info =
	Reflect<std::optional<BaseType>>::ThisTypeInfo{ "std::optional"sv }
	.Description("optional");
