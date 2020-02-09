#pragma once
#include <array>
#include <string_view>
#include <vector>
#include <type_traits>
#include "Engine/ArrayView.h"
#include "Engine/Hash.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
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
	/** A view type specialized for field components */
	template<typename T>
	struct FieldView : TArrayView<T const*> {
		T const* Find(Hash32 id) const {
			//@todo If we have a way to ensure that the field array is sorted, we can use a binary search here for better speed.
			const auto iter = std::find_if(this->begin(), this->end(), [=](T const* info) { return info->id == id; });
			if (iter != this->end()) return *iter;
			else return nullptr;
		}
	};

	/** Views into various field types that the struct defines */
	struct Fields {
		FieldView<ConstantInfo> constants;
		FieldView<VariableInfo> variables;
	};

	/** Info for a struct type, which contains various fields and supports inheritance */
	struct StructTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Struct;

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo const* baseType = nullptr;
		/** A default-constructed instance of this struct type, used to find default values for variables */
		void const* defaults = nullptr;
		/** The static fields that this type defines */
		Fields statics;
		/** The member fields that this type defines */
		Fields members;

		StructTypeInfo() = delete;
		StructTypeInfo(
			Hash128 inID, CompilerDefinition inDefinition,
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			StructTypeInfo const* inBaseType, void const* inDefaults,
			Fields inStatics, Fields inMembers
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
			Fields inStatics, Fields inMembers)
		: StructTypeInfo(
			TypeResolver<StructType>::GetID(), GetCompilerDefinition<StructType>(),
			inDescription, inFlags, inSerializer,
			Reflection::Cast<StructTypeInfo>(TypeResolver<BaseType>::Get()), inDefaults,
			inStatics, inMembers)
		{}

		STANDARD_TYPEINFO_METHODS(StructType)
	};
}
