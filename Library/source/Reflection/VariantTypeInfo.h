#pragma once
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct VariantTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Variant;

		/** The number of possible types this variant can have */
		size_t possibleTypeCount = 0;

		VariantTypeInfo() = delete;
		VariantTypeInfo(
			Hash128 inID, CompilerDefinition inDef,
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			size_t inPossibleTypeCount
		);
		virtual ~VariantTypeInfo() = default;

		/** Returns the type info for the possible type at the specified index */
		virtual TypeInfo const* GetPossibleType(size_t index) const = 0;
		/** Returns true if the specified type is one of the possible types for this variant */
		virtual bool CanContain(TypeInfo const* type) const = 0;

		/** Returns the current type of the variant */
		virtual TypeInfo const* GetType(void const* instance) const = 0;

		/** Returns the value in the variant */
		virtual void* GetValue(void* instance) const = 0;
		virtual void const* GetValue(void const* instance) const = 0;

		/** Assign the value inside the variant. Returns true if the new value was successfully assigned */
		virtual bool Assign(void* instance, const TypeInfo* type, void const* value) = 0;
		/** Return the variant to an unintialized state, if possible */
		virtual bool UnAssign(void* instance) = 0;
	};

	template<typename OptionalType, typename ValueType>
	struct TOptionalTypeInfo : public VariantTypeInfo {
		TOptionalTypeInfo()
		: VariantTypeInfo(TypeResolver<OptionalType>::GetID(), GetCompilerDefinition<OptionalType>())
		{}

		STANDARD_TYPEINFO_METHODS(OptionalType)

		static constexpr ValueType const& CastValue(void const* value) { return *static_cast<ValueType const*>(value); };

		virtual TypeInfo const* GetPossibleType( size_t index ) const final {
			switch(index) {
				case 0: return TypeResolver<void>::Get();
				case 1: return TypeResolver<ValueType>::Get();
				default: return nullptr;
			}
		};
		virtual bool CanContain(TypeInfo const* type) const final {
			return type == TypeResolver<void>::Get() || type == TypeResolver<ValueType>::Get();
		}

		virtual TypeInfo const* GetType(void const* instance) const {
			if (Cast(instance)) return TypeResolver<ValueType>::Get();
			else return TypeResolver<void>::Get();
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

		virtual bool Assign(void* instance, TypeInfo const* type, void const* value) const final {
			if (type == TypeResolver<ValueType>::Get()) {
				Cast(instance).emplace(CastValue(value));
				return true;
			} else {
				return false;
			}
		}
		virtual bool Unassign(void* instance) const final { Cast(instance).reset(); return true; }
	};
}
