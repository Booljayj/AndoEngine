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

	struct StructVariableRange {
		struct Iterator {
			using difference_type = std::ptrdiff_t;
			using value_type = VariableInfo const;

			constexpr Iterator() = default;
			Iterator(Iterator const&) = default;
			Iterator(Iterator&&) = default;
			Iterator(StructTypeInfo const& type);

			Iterator& operator=(Iterator const&) = default;
			Iterator& operator=(Iterator&&) = default;

			friend inline bool operator==(const Iterator& a, const Iterator& b) { return a.current == b.current && a.index == b.index; };
			friend inline bool operator!=(const Iterator& a, const Iterator& b) { return !(a == b); };

			inline operator bool() const { return current != nullptr; }
			Iterator& operator++();
			Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

			VariableInfo const& operator*() const;
			VariableInfo const* operator->() const;

		private:
			size_t index = 0;
			StructTypeInfo const* current = nullptr;
		};

		StructVariableRange(StructTypeInfo const& type) : type(type) {}

		inline Iterator begin() const noexcept { return Iterator{ type }; }
		inline constexpr Iterator end() const noexcept { return Iterator{}; }

		/** Finds a variable with the given name */
		VariableInfo const* FindVariable(std::string_view name) const;
		/** Finds a variable with the given id */
		VariableInfo const* FindVariable(Hash32 id) const;

	private:
		StructTypeInfo const& type;
	};
	
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

		static void SerializeVariables(StructTypeInfo const& type, YAML::Node& node, void const* instance);
		static void DeserializeVariables(StructTypeInfo const& type, YAML::Node const& node, void* instance);

		static void SerializeVariables(StructTypeInfo const& type, Archive::Output& archive, void const* instance);
		static void DeserializeVariables(StructTypeInfo const& type, Archive::Input& archive, void* instance);

		bool IsChildOf(StructTypeInfo const& target) const {
			//Walk up the chain of parents until we encounter the provided type
			for (StructTypeInfo const* current = this; current; current = current->base) {
				if (current == &target) return true;
			}
			return false;
		}

		template<Concepts::ReflectedStruct T>
		bool IsChildOf() const { return IsChildOf(Reflect<T>::Get()); }

		/** Create a range object used to iterate through or search the variables on this type */
		StructVariableRange GetVariableRange() const { return StructVariableRange{ *this }; }

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

namespace Archive {
	template<Reflection::Concepts::ReflectedStruct Type>
	struct Serializer<Type> {
		void Write(Output& archive, Type const& instance) {
			Reflection::StructTypeInfo::SerializeVariables(Reflect<Type>::Get(), archive, &instance);
		}
		void Read(Input& archive, Type& instance) {
			Reflection::StructTypeInfo::DeserializeVariables(Reflect<Type>::Get(), archive, &instance);
		}
	};
}

namespace YAML {
	/** Default converter for struct types. If a more explicit converter is available, that should be used instead of this. */
	template<Reflection::Concepts::ReflectedStruct Type>
	struct convert<Type> {
		static Node encode(const Type& instance) {
			Node node{ NodeType::Map };
			Reflection::StructTypeInfo::SerializeVariables(Reflect<Type>::Get(), node, &instance);
			return node;
		}
		static bool decode(const Node& node, Type& instance) {
			if (!node.IsMap()) return false;
			Reflection::StructTypeInfo::DeserializeVariables(Reflect<Type>::Get(), node, &instance);
			return true;
		}
	};
}
