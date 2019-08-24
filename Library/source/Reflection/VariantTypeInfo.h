#pragma once
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

// std::optional is not yet widely supported until c++17, so it can remain dormant.
#define STD_OPTIONAL_SUPPORT 0
#if STD_OPTIONAL_SUPPORT
#include <optional>
#endif

/** @experimental
 * Variants are not well-supported yet in most C++ compilers, so this implementation is incomplete.
 * Once they are more regularly supported, this implementation should be completed. It may require
 * some significant changes to fully work once that happens, but that's a problem for Future Justin.
 */

namespace Reflection {
	struct VariantTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Variant;

		/** The number of possible types this variant can have */
		size_t PossibleTypeCount = 0;

		VariantTypeInfo() = delete;
		VariantTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			size_t InPossibleTypeCount
		);
		virtual ~VariantTypeInfo() = default;

		/** Returns the type info for the possible type at the specified index */
		virtual TypeInfo const* GetPossibleType(size_t Index) const = 0;
		/** Returns true if the specified type is one of the possible types for this variant */
		virtual bool CanContain(TypeInfo const* Info) const = 0;

		/** Returns the current type of the variant */
		virtual TypeInfo const* GetType(void const* Instance) const = 0;

		/** Returns the value in the variant */
		virtual void* GetValue(void* Instance) const = 0;
		virtual void const* GetValue(void const* Instance) const = 0;

		/** Assign the value inside the variant. Returns true if the new value was successfully assigned */
		virtual bool Assign(void* Instance, const TypeInfo* Info, void const* Value) = 0;
		/** Return the variant to an unintialized state, if possible */
		virtual bool UnAssign(void* Instance) = 0;
	};

	//@note This requires some visitor templates similar to TTupleTypeInfo to fully work.
	// template<TVARIANT, TTYPES...>
	// struct TVariantTypeInfo : public VariantTypeInfo {
	// };

#if STD_OPTIONAL_SUPPORT
	template<typename ValueType>
	struct TOptionalTypeInfo : public VariantTypeInfo {
		using OptionalType = std::optional<ValueType>;

		TOptionalTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: VariantTypeInfo(
			TypeResolver<OptionalType>::GetID(), GetCompilerDefinition<OptionalType>(),
			InDescription, InFlags, InSerializer
			/***/ )
		{}

		STANDARD_TYPEINFO_METHODS(OptionalType)

		static constexpr ValueType const& CastValue(void const* Value) { return *static_cast<ValueType const*>(Value); };

		virtual TypeInfo const* GetPossibleType( size_t Index ) const final {
			switch(Index) {
				case 0: return TypeResolver<void>::Get();
				case 1: return TypeResolver<ValueType>::Get();
				default: return nullptr;
			}
		};
		virtual bool CanContain(TypeInfo const* Info) const final { return Info == TypeResolver<void>::Get() || Info == TypeResolver<ValueType>::Get(); }

		virtual TypeInfo const* GetType(void const* Instance) const {
			if (Cast(Instance)) return TypeResolver<ValueType>::Get();
			else return TypeResolver<void>::Get();
		}

		virtual void* GetValue(void* Instance) const final {
			OptionalType& Opt = Cast(Instance);
			if (Opt) return &Opt.value();
			else return nullptr;
		};
		virtual void const* GetValue(void const* Instance) const final {
			OptionalType const& Opt = Cast(Instance);
			if (Opt) return &Opt.value();
			else return nullptr;
		};

		virtual bool Assign(void* Instance, TypeInfo const* Info, void const* Value) const final {
			if (Info == TypeResolver<ValueType>::Get()) {
				Cast(Instance).emplace(CastValue(Value));
				return true;
			} else {
				return false;
			}
		}
		virtual bool Unassign(void* Instance) const final { Cast(Instance).reset(); return true; }
	};
#endif
}

#undef STD_OPTIONAL_SUPPORT
