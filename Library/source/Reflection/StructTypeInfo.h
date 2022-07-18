#pragma once
#include "Engine/ArrayView.h"
#include "Engine/Hash.h"
#include "Engine/STL.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

/** Define members of the struct used for reflection. The second argument must be either the primary baseType class of this type or void */
#define DECLARE_REFLECTION_MEMBERS(StructType_, BaseType_)\
using BaseType = BaseType_;\
virtual ~StructType_() = default;\
virtual ::Reflection::StructTypeInfo const& GetTypeInfo() const { return typeInfo_; }\
static ::Reflection::StructTypeInfo const& TypeInfo() { return typeInfo_; }\
static ::Reflection::TStructTypeInfo<StructType_> const typeInfo_

#define DEFINE_DECLARE_REFLECTION_MEMBERS(StructType_)\
::Reflection::TStructTypeInfo<StructType_> const StructType_::typeInfo_ = ::Reflection::TStructTypeInfo<StructType_>{}

/** Expose a struct to the reflection system. Must be used after the struct is declared. Struct must include the DECLARE_REFLECTION_MEMBERS macro. */
#define REFLECT(StructType_)\
namespace Reflection {\
	template<> struct TypeResolver<StructType_> {\
		static StructTypeInfo const* Get() { return &StructType_::typeInfo_; }\
		static constexpr Hash128 GetID() { return Hash128{ #StructType_ }; }\
	};\
}

namespace Reflection {
	/** Info for a struct type, which contains various fields and supports inheritance */
	struct StructTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Struct;

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo const* baseType = nullptr;
		/** A default-constructed instance of this struct type, used to find default values for variables */
		void const* defaults = nullptr;
		/** The variables that are contained in this type */
		TArrayView<VariableInfo const> variables;

		StructTypeInfo() = delete;
		StructTypeInfo(Hash128 inID, CompilerDefinition inDefinition);
		virtual ~StructTypeInfo() = default;

		/** Returns true if the chain of baseType types includes the provided type */
		bool DerivesFrom(TypeInfo const* other) const { return true; }
	};

	//============================================================
	// Templates

	template<typename StructType_>
	struct TStructTypeInfo : public StructTypeInfo {
		using StructType = StructType_;

		TStructTypeInfo()
		: StructTypeInfo(TypeResolver<StructType>::GetID(), GetCompilerDefinition<StructType>())
		{}

		STANDARD_TYPEINFO_METHODS(StructType)

		template<typename Type>
		inline TStructTypeInfo& BaseType() {
			static_assert(std::is_base_of_v<Type, StructType>, "invalid inheritance for StructTypeInfo, Type is not a base of T");
			static_assert(std::is_class_v<Type>, "invalid inheritance for StructTypeInfo, Type is not a class");
			baseType = Reflection::Cast<StructTypeInfo>(TypeResolver<Type>::Get());
			return *this;
		}
		inline TStructTypeInfo& Defaults(void const* inDefaults) { defaults = inDefaults; return *this; }
		inline TStructTypeInfo& Variables(TArrayView<VariableInfo const> inVariables) { variables = std::move(inVariables); return *this; }
	};
}
