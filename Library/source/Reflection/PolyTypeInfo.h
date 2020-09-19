#pragma once
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	//@note "Poly" is short for Polymorphic Interface. A poly holds an optional instance that can be cast to a base type.

	struct PolyTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Poly;

		/** The base type for this poly */
		TypeInfo const* baseType = nullptr;
		/** Whether this poly can hold a value of the base type */
		uint8_t canBeBaseType : 1;
		/** Whether this poly can hold a value that derives from the base type */
		uint8_t canBeDerivedType : 1;

		PolyTypeInfo() = delete;
		PolyTypeInfo(Hash128 inID, CompilerDefinition inDef);
		virtual ~PolyTypeInfo() = default;

		bool CanAssignType(TypeInfo const* type) const;

		/** Get the current value of the poly. Can be nullptr if the poly is unassigned */
		virtual void* GetValue(void* instance) const = 0;
		virtual void const* GetValue(void const* instance) const = 0;

		/** Assign a new value to a poly */
		virtual bool Assign(void* instance, TypeInfo const* type, void const* value) const = 0;
		/** Unassign the value of a poly */
		virtual void Unassign(void* instance) const = 0;
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

		TUniquePtrTypeInfo()
		: PolyTypeInfo(TypeResolver<PointerType>::GetID(), GetCompilerDefinition<PointerType>())
		{
			baseType = TypeResolver<BaseType>::Get();
			canBeBaseType = !std::is_abstract<BaseType>::value;
			canBeDerivedType = std::is_class<BaseType>::value && !std::is_final<BaseType>::value;
		}

		STANDARD_TYPEINFO_METHODS(PointerType)

		virtual void* GetValue(void* instance) const final { return Cast(instance).get(); }
		virtual void const* GetValue(void const* instance) const final { return Cast(instance).get(); }

		virtual bool Assign(void* instance, TypeInfo const* type, void const* value) const final {
			if (CanAssignType(this, type)) {
				void* newInstance = std::malloc(type->def.size);
				if (value) type->Construct(newInstance, value);
				else type->Construct(newInstance);
				Cast(instance).reset(reinterpret_cast<BaseType*>(newInstance));
				return true;
			} else {
				return false;
			}
		}
		virtual void Unassign(void* instance) const final { Cast(instance).reset(); }
	};
}
