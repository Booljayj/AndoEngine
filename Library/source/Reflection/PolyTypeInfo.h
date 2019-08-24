#pragma once
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	//@note "Poly" is short for Polymorphic Interface. A poly holds an optional instance that can be cast to a base type.

	struct PolyTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Poly;

		/** The base type for this poly */
		TypeInfo const* BaseTypeInfo = nullptr;
		/** Whether this poly can hold a value of the base type */
		uint8_t CanBeBaseType : 1;
		/** Whether this poly can hold a value that derives from the base type */
		uint8_t CanBeDerivedType : 1;

		PolyTypeInfo() = delete;
		PolyTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TypeInfo const* InBaseTypeInfo, bool InCanBeBaseType, bool InCanBeDerivedType
		);
		virtual ~PolyTypeInfo() = default;

		static bool CanAssignType(PolyTypeInfo const* PolyInfo, TypeInfo const* Info);

		/** Get the current value of the poly. Can be nullptr if the poly is unassigned */
		virtual void* GetValue(void* Instance) const = 0;
		virtual void const* GetValue(void const* Instance) const = 0;

		/** Assign a new value to a poly */
		virtual bool Assign(void* Instance, TypeInfo const* Type, void const* Value) const = 0;
		/** Unassign the value of a poly */
		virtual void Unassign(void* Instance) const = 0;
	};

	//============================================================
	// Templates

	template<typename BaseType>
	struct TUniquePtrTypeInfo : public PolyTypeInfo {
		static_assert(
			!std::is_void<BaseType>::value,
			"std::unique_ptr<void> is not supported for reflection. Reflecting the internal value is inherently impossible, which makes it unsafe to use."
		);
		using PointerType = std::unique_ptr<BaseType>;

		TUniquePtrTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: PolyTypeInfo(
			TypeResolver<PointerType>::GetID(), GetCompilerDefinition<PointerType>(),
			InDescription, InFlags, InSerializer,
			TypeResolver<BaseType>::Get(), !std::is_abstract<BaseType>::value, std::is_class<BaseType>::value && !std::is_final<BaseType>::value)
		{}

		STANDARD_TYPEINFO_METHODS(PointerType)

		virtual void* GetValue(void* Instance) const final { return Cast(Instance).get(); }
		virtual void const* GetValue(void const* Instance) const final { return Cast(Instance).get(); }

		virtual bool Assign(void* Instance, TypeInfo const* Type, void const* Value) const final {
			if (CanAssignType(this, Type)) {
				void* NewInstance = std::malloc(Type->Definition.Size);
				if (Value) Type->Construct(NewInstance, Value);
				else Type->Construct(NewInstance);
				Cast(Instance).reset(reinterpret_cast<BaseType*>(NewInstance));
				return true;
			} else {
				return false;
			}
		}
		virtual void Unassign(void* Instance) const final { Cast(Instance).reset(); }
	};
}
