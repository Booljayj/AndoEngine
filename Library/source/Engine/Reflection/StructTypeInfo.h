#pragma once
#include "Engine/Reflection/Components/VariableInfo.h"
#include "Engine/Reflection/ReflectionMacros.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	/** Info for a struct type, which contains various fields and supports inheritance */
	struct StructTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Struct;

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo const* baseType = nullptr;
		/** A default-constructed instance of this struct type, used to find default values for variables */
		void const* defaults = nullptr;
		/** The variables that are contained in this type */
		std::vector<VariableInfo> variables;

		virtual ~StructTypeInfo() = default;

		/** Returns true if the chain of baseType types includes the provided type */
		bool DerivesFrom(TypeInfo const* other) const {
			if (this == other) return true;
			else if (baseType) return baseType->DerivesFrom(other);
			else return false;
		}

		/** Finds a variable with the given id */
		VariableInfo const* FindVariable(Hash32 id) const {
			const auto iter = std::find_if(variables.begin(), variables.end(), [id](VariableInfo const& info) { return info.id == id; });
			if (iter != variables.end()) return &(*iter);
			return nullptr;
		}
	};

	//============================================================
	// Templates

	template<typename StructType_>
	struct TStructTypeInfo : public ImplementedTypeInfo<StructType_, StructTypeInfo> {
		using StructType = StructType_;
		using StructTypeInfo::baseType;
		using StructTypeInfo::defaults;
		using StructTypeInfo::variables;

		TStructTypeInfo(std::string_view inName)
		: ImplementedTypeInfo<StructType_, StructTypeInfo>(Reflect<StructType>::ID, inName)
		{}

		TYPEINFO_BUILDER_METHODS(StructType)

		template<typename Type>
		inline decltype(auto) BaseType() {
			static_assert(std::is_same_v<Type, void> || std::is_base_of_v<Type, StructType>, "invalid inheritance for StructTypeInfo, Type is not a base of T");
			static_assert(std::is_same_v<Type, void> || std::is_class_v<Type>, "invalid inheritance for StructTypeInfo, Type is not a class");
			if constexpr (!std::is_same_v<Type, void>) baseType = ::Reflection::Cast<StructTypeInfo>(Reflect<Type>::Get());
			return *this;
		}
		inline decltype(auto) Defaults(void const* inDefaults) { defaults = inDefaults; return *this; }
		inline decltype(auto) Variables(std::initializer_list<VariableInfo>&& inVariables) { variables = inVariables; return *this; }
	};
}

TYPEINFO_REFLECT(Struct);

//============================================================
// Standard struct reflection

REFLECT(glm::vec2, Struct);
REFLECT(glm::vec3, Struct);
REFLECT(glm::vec4, Struct);

REFLECT(glm::u32vec2, Struct);
REFLECT(glm::u32vec3, Struct);

REFLECT(glm::i32vec2, Struct);
REFLECT(glm::i32vec3, Struct);

REFLECT(glm::u8vec3, Struct);
REFLECT(glm::u8vec4, Struct);

REFLECT(glm::mat2x2, Struct);
REFLECT(glm::mat2x3, Struct);
REFLECT(glm::mat2x4, Struct);

REFLECT(glm::mat3x2, Struct);
REFLECT(glm::mat3x3, Struct);
REFLECT(glm::mat3x4, Struct);

REFLECT(glm::mat4x2, Struct);
REFLECT(glm::mat4x3, Struct);
REFLECT(glm::mat4x4, Struct);
