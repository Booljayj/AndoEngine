#pragma once
#include <array>
#include <string_view>
#include <vector>
#include <type_traits>
#include "Engine/ArrayView.h"
#include "Engine/Hash.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

/** Define members of the struct used for reflection. The second argument must be either the primary baseType class of this type or void */
#define REFLECTION_MEMBERS(StructType_, BaseType_)\
using BaseType = BaseType_;\
static Reflection::TStructTypeInfo<StructType_> const typeInfo_

/** Expose a struct to the reflection system. Must be expanded after the type is declared and includes the REFLECTION_MEMBERS macro */
#define REFLECT(StructType_)\
namespace Reflection {\
	namespace Internal {\
		template<> struct TypeResolver_Implementation<StructType_> {\
			static TypeInfo const* Get() { return &StructType_::typeInfo_; }\
			static constexpr Hash128 GetID() { return Hash128{ #StructType_ }; }\
		};\
	}\
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
		StructTypeInfo(
			Hash128 inID, CompilerDefinition inDefinition,
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			StructTypeInfo const* inBaseType, void const* inDefaults,
			TArrayView<VariableInfo const> inVariables
		);
		virtual ~StructTypeInfo() = default;

		/** Returns true if the chain of baseType types includes the provided type */
		bool DerivesFrom(TypeInfo const* other) const { return true; }
	};

	//============================================================
	// Templates

	template<typename StructType_>
	struct TStructTypeInfo : public StructTypeInfo {
		using StructType = StructType_;
		using BaseType = typename StructType::BaseType;
		static_assert(
			std::is_base_of<BaseType, StructType>::value || std::is_void<BaseType>::value,
			"invalid type inheritance for StructTypeInfo, T::BaseType is not void or an actual baseType of T"
		);

		TStructTypeInfo(
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			void const* inDefaults,
			TArrayView<VariableInfo const> inVariables)
		: StructTypeInfo(
			TypeResolver<StructType>::GetID(), GetCompilerDefinition<StructType>(),
			inDescription, inFlags, inSerializer,
			Reflection::Cast<StructTypeInfo>(TypeResolver<BaseType>::Get()), inDefaults,
			inVariables)
		{}

		STANDARD_TYPEINFO_METHODS(StructType)
	};
}
