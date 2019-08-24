#pragma once
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	template<typename Type>
	struct TPrimitiveTypeInfo : public TypeInfo {
		TPrimitiveTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: TypeInfo(
			TypeInfo::CLASSIFICATION, TypeResolver<Type>::GetID(), GetCompilerDefinition<Type>(),
			InDescription, InFlags, InSerializer)
		{}

		STANDARD_TYPEINFO_METHODS(Type)
	};

	/** Special type for void, which is not a type that can have values but still needs TypeInfo */
	template<>
	struct TPrimitiveTypeInfo<void> : public TypeInfo {
		TPrimitiveTypeInfo()
		: TypeInfo(
			TypeInfo::CLASSIFICATION, Hash128{}, GetCompilerDefinition<void>(),
			"not a type", FTypeFlags::None, nullptr)
		{}

		virtual void Construct(void* I) const final {}
		virtual void Construct(void* I, void const* T) const final {}
		virtual void Destruct(void* I) const final {}
		virtual bool Equal(void const* A, void const* B) const final { return false; }
	};
}
