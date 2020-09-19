#pragma once
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	template<typename Type>
	struct TPrimitiveTypeInfo : public TypeInfo {
		TPrimitiveTypeInfo()
		: TypeInfo(TypeInfo::Classification, TypeResolver<Type>::GetID(), GetCompilerDefinition<Type>())
		{}

		STANDARD_TYPEINFO_METHODS(Type)
	};

	/** Special type for void, which is not a type that can have values but still needs TypeInfo */
	template<>
	struct TPrimitiveTypeInfo<void> : public TypeInfo {
		TPrimitiveTypeInfo()
		: TypeInfo(TypeInfo::Classification, Hash128{}, GetCompilerDefinition<void>())
		{
			description = "not a type";
		}

		virtual void Construct(void* instance) const final {}
		virtual void Construct(void* instance, void const* other) const final {}
		virtual void Destruct(void* instance) const final {}
		virtual bool Equal(void const* instanceA, void const* instanceB) const final { return false; }
	};
}
