#pragma once
#include "Engine/Reflection/Components/VariableInfo.h"
#include "Engine/Reflection/ReflectionMacros.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	struct StructTypeInfo;

	namespace Concepts {
		template<typename T>
		concept ReflectedStruct = ReflectedType<T> and requires (T a) {
			{ ::Reflection::Reflect<T>::Get() } -> std::convertible_to<StructTypeInfo const&>;
		};

		template<typename T>
		concept ReflectedHierarchicalStruct = ReflectedStruct<T> and requires(T a) {
			{ a.GetTypeInfo() } -> std::convertible_to<StructTypeInfo const&>;
		};

		template<typename T, typename StructType>
		concept ReflectedStructBase =
			std::is_same_v<T, void> or
			(std::derived_from<StructType, T> and std::is_class_v<T>);
	}

	/** Info for a struct type, which contains various fields and supports inheritance */
	struct StructTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Struct;

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo const* base = nullptr;
		/** A default-constructed instance of this struct type, used to find default values for variables */
		void const* defaults = nullptr;
		/** The variables that are contained in this type */
		std::vector<VariableInfo> variables;

		virtual ~StructTypeInfo() = default;

		bool IsChildOf(StructTypeInfo const& target) const {
			//Walk up the chain of parents until we encounter the provided type
			for (StructTypeInfo const* current = this; current; current = current->base) {
				if (current == &target) return true;
			}
			return false;
		}

		template<Concepts::ReflectedStruct T>
		bool IsChildOf() const { return IsChildOf(Reflect<T>::Get()); }

		/** Finds a variable with the given id */
		VariableInfo const* FindVariable(Hash32 id) const {
			const auto iter = std::find_if(variables.begin(), variables.end(), [id](VariableInfo const& info) { return info.id == id; });
			if (iter != variables.end()) return &(*iter);
			return nullptr;
		}

		/** Returns the known specific type of an instance deriving from this type, to our best determination. */
		virtual StructTypeInfo const* GetDerivedTypeInfo(void const* instance) const = 0;
	};

	//============================================================
	// Templates

	template<typename StructType_>
		requires std::is_class_v<StructType_>
	struct TStructTypeInfo : public ImplementedTypeInfo<StructType_, StructTypeInfo> {
		using StructType = StructType_;
		using StructTypeInfo::base;
		using StructTypeInfo::defaults;
		using StructTypeInfo::variables;
		using ImplementedTypeInfo<StructType, StructTypeInfo>::Cast;

		TStructTypeInfo(std::string_view name) : ImplementedTypeInfo<StructType, StructTypeInfo>(::Reflection::Reflect<StructType>::ID, name) {}

		virtual StructTypeInfo const* GetDerivedTypeInfo(void const* instance) const final {
			if constexpr (Concepts::ReflectedHierarchicalStruct<StructType>) return &Cast(instance).GetTypeInfo();
			else return this;
		}

		TYPEINFO_BUILDER_METHODS(StructType)
		template<Concepts::ReflectedStructBase<StructType> BaseType>
		inline decltype(auto) Base() {
			if constexpr (!std::is_same_v<BaseType, void>) base = &Reflect<BaseType>::Get();
			return *this;
		}
		inline decltype(auto) Defaults(void const* inDefaults) { defaults = inDefaults; return *this; }
		inline decltype(auto) Variables(std::initializer_list<VariableInfo>&& inVariables) { variables = inVariables; return *this; }
	};
}

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
