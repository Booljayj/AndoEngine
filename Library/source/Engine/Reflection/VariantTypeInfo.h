#pragma once
#include "Engine/Reflection/TypeInfo.h"
#include "Engine/VariantUtility.h"

namespace Reflection {
	/** TypeInfo for a variant type. Variants are any type that can hold a single value with more than one possible type. */
	struct VariantTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Variant;

		/** The possible types this variant can have */
		std::vector<TypeInfo const*> types;

		virtual ~VariantTypeInfo() = default;

		/** Returns true if the specified type is one of the possible types for this variant */
		inline bool CanContain(TypeInfo const* type) const {
			return std::find(types.begin(), types.end(), type) != types.end();
		}

		/** Returns the current type the variant is holding */
		virtual TypeInfo const* GetType(void const* instance) const = 0;

		/** Returns the value in the variant */
		virtual void* GetValue(void* instance) const = 0;
		virtual void const* GetValue(void const* instance) const = 0;

		/** Assign the value inside the variant. Returns true if the new value was successfully assigned. Value is optional, if provided the newly assigned value will be a copy */
		virtual bool Assign(void* instance, const TypeInfo* type, void const* source) const = 0;
	};

	//============================================================
	// Templates

	struct ReflectVisitor {
		template<typename Type>
		TypeInfo const* operator()(Type const&) { return Reflect<Type>::Get(); }
	};

	template<typename VariantType, typename... ValueTypes>
	struct TVariantTypeInfo : public ImplementedTypeInfo<VariantType, VariantTypeInfo> {
		using ImplementedTypeInfo<VariantType, VariantTypeInfo>::Cast;
		using VariantTypeInfo::types;

		TVariantTypeInfo(std::string_view inName) : ImplementedTypeInfo<VariantType, VariantTypeInfo>(Reflect<VariantType>::ID, inName) {
			types = { Reflect<ValueTypes>::Get() ... };
		}

		virtual TypeInfo const* GetType(void const* instance) const {
			return std::visit(ReflectVisitor{}, Cast(instance));
		}

		virtual void* GetValue(void* instance) const final {
			return std::visit(VariantUtility::PointerVisitor{}, Cast(instance));
		};
		virtual void const* GetValue(void const* instance) const final {
			return std::visit(VariantUtility::PointerVisitor{}, Cast(instance));
		};

		virtual bool Assign(void* instance, TypeInfo const* type, void const* source) const final {
			const auto iter = std::find(types.begin(), types.end(), type);
			if (iter != types.end()) {
				const size_t index = (iter - types.begin());
				if (source) VariantUtility::EmplaceIndex(Cast(instance), index, source);
				else VariantUtility::EmplaceIndex(Cast(instance), index);
				return true;
			}
			return false;
		}
	};

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

		virtual TypeInfo const* GetType(void const* instance) const {
			if (Cast(instance)) return &Reflect<ValueType>::Get();
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

		virtual bool Assign(void* instance, TypeInfo const* type, void const* source) const final {
			if (type == &Reflect<ValueType>::Get()) {
				if (source) Cast(instance).emplace(CastValue(source));
				else Cast(instance).emplace();
				return true;
			} else if (type == &Reflect<void>::Get()) {
				Cast(instance).reset();
				return true;
			} else {
				return false;
			}
		}
	};

	//============================================================
	// Standard variant reflection

	template<typename BaseType>
	struct Reflect<std::optional<BaseType>> {
		static VariantTypeInfo const& Get() { return info; }
		static constexpr Hash128 ID = Hash128{ "std::optional"sv } + Reflect<BaseType>::ID;
	private:
		using ThisTypeInfo = TOptionalTypeInfo<std::optional<BaseType>, BaseType>;
		static ThisTypeInfo const info;
	};
	template<typename BaseType>
	typename Reflect<std::optional<BaseType>>::ThisTypeInfo const Reflect<std::optional<BaseType>>::info =
		Reflect<std::optional<BaseType>>::ThisTypeInfo{ "std::optional"sv }
		.Description("optional type");
}
