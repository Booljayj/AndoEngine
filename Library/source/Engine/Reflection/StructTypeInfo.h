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

		bool IsChildOf(StructTypeInfo const& parent) const {
			//Walk up the chain of parents until we encounter the provided type
			for (StructTypeInfo const* current = this; current; current = current->base) {
				if (current == &parent) return true;
			}
			return false;
		}

		template<Concepts::ReflectedStruct T>
		bool IsChildOf() const { return IsChildOf(Reflect<T>::Get()); }

		/** Create a range object used to iterate through or search the variables on this type */
		StructVariableRange GetVariableRange() const { return StructVariableRange{ *this }; }

		/** Returns the known specific type of an instance deriving from this type, to our best determination. */
		virtual StructTypeInfo const* GetDerivedTypeInfo(void const* instance) const = 0;

		/** Builder method to add a defaults instance to a struct type */
		template<typename Self>
		inline auto& Defaults(this Self&& self, void const* inDefaults) { self.defaults = inDefaults; return self; }
		/** Builder method to add variables to a struct type */
		template<typename Self>
		inline auto& Variables(this Self&& self, std::initializer_list<VariableInfo>&& inVariables) { self.variables = inVariables; return self; }
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

		template<Concepts::ReflectedStructBase<StructType> BaseType>
		inline decltype(auto) Base() {
			if constexpr (!std::is_same_v<BaseType, void>) base = &Reflect<BaseType>::Get();
			return *this;
		}
	};
}

//============================================================
// Standard struct reflection

/** Define serialization methods for a type which are based on the reflection system */
#define DEFINE_REFLECTED_SERIALIZATION(StructType) \
namespace Archive {\
	template<> struct Serializer<StructType> {\
		void Write(Output& archive, StructType const& instance) { Reflection::StructTypeInfo::SerializeVariables(::Reflection::Reflect<StructType>::Get(), archive, &instance); }\
		void Read(Input& archive, StructType& instance) { Reflection::StructTypeInfo::DeserializeVariables(::Reflection::Reflect<StructType>::Get(), archive, &instance); }\
	};\
}\
namespace YAML {\
	template<> struct convert<StructType> {\
		static Node encode(const StructType& instance) {\
			Node node{ NodeType::Map }; Reflection::StructTypeInfo::SerializeVariables(::Reflection::Reflect<StructType>::Get(), node, &instance); return node;\
		}\
		static bool decode(const Node& node, StructType& instance) {\
			if (!node.IsMap()) return false;\
			Reflection::StructTypeInfo::DeserializeVariables(::Reflection::Reflect<StructType>::Get(), node, &instance); return true;\
		}\
	};\
}

namespace Reflection {
	template<typename FirstType, typename SecondType>
	struct Reflect<std::pair<FirstType, SecondType>> {
		static StructTypeInfo const& Get() { return info; }
		static constexpr Hash128 ID = Hash128{ "std::pair"sv } + Reflect<FirstType>::ID + Reflect<SecondType>::ID;
	private:
		using ThisTypeInfo = TStructTypeInfo<std::pair<FirstType, SecondType>>;
		static ThisTypeInfo const info;

		static void const* GetDefaults() {
			if constexpr (std::is_default_constructible_v<FirstType> && std::is_default_constructible_v<SecondType>) {
				static std::pair<FirstType, SecondType> const defaults;
				return &defaults;
			} else {
				return nullptr;
			}
		}
	};

	template<typename FirstType, typename SecondType>
	typename Reflect<std::pair<FirstType, SecondType>>::ThisTypeInfo const Reflect<std::pair<FirstType, SecondType>>::info =
		typename Reflect<std::pair<FirstType, SecondType>>::ThisTypeInfo{ "std::pair"sv }
		.Description("pair"sv)
		.Defaults(Reflect<std::pair<FirstType, SecondType>>::GetDefaults())
		.Variables({
			{ &std::pair<FirstType, SecondType>::first, "first"sv, "first value in pair"sv, FVariableFlags::None() },
			{ &std::pair<FirstType, SecondType>::second, "second"sv, "second value in pair"sv, FVariableFlags::None() },
		});
}

namespace Archive {
	template<typename FirstType, typename SecondType>
	struct Serializer<std::pair<FirstType, SecondType>> {
		void Write(Output& archive, std::pair<FirstType, SecondType> const& instance) {
			Serializer<FirstType>::Write(archive, instance.first);
			Serializer<FirstType>::Write(archive, instance.second);
		}
		void Read(Input& archive, std::pair<FirstType, SecondType>& instance) {
			Serializer<FirstType>::Read(archive, instance.first);
			Serializer<FirstType>::Read(archive, instance.second);
		}
	};
}
//YAML serialization methods for std::pair are already defined in the YAML library

REFLECT(glm::vec2, Struct);
REFLECT(glm::vec3, Struct);
REFLECT(glm::vec4, Struct);
DEFINE_REFLECTED_SERIALIZATION(glm::vec2);
DEFINE_REFLECTED_SERIALIZATION(glm::vec3);
DEFINE_REFLECTED_SERIALIZATION(glm::vec4);

REFLECT(glm::u32vec2, Struct);
REFLECT(glm::u32vec3, Struct);
DEFINE_REFLECTED_SERIALIZATION(glm::u32vec2);
DEFINE_REFLECTED_SERIALIZATION(glm::u32vec3);

REFLECT(glm::i32vec2, Struct);
REFLECT(glm::i32vec3, Struct);
DEFINE_REFLECTED_SERIALIZATION(glm::i32vec2);
DEFINE_REFLECTED_SERIALIZATION(glm::i32vec3);

REFLECT(glm::u8vec3, Struct);
REFLECT(glm::u8vec4, Struct);
DEFINE_REFLECTED_SERIALIZATION(glm::u8vec3);
DEFINE_REFLECTED_SERIALIZATION(glm::u8vec4);

REFLECT(glm::mat2x2, Struct);
REFLECT(glm::mat2x3, Struct);
REFLECT(glm::mat2x4, Struct);
DEFINE_REFLECTED_SERIALIZATION(glm::mat2x2);
DEFINE_REFLECTED_SERIALIZATION(glm::mat2x3);
DEFINE_REFLECTED_SERIALIZATION(glm::mat2x4);

REFLECT(glm::mat3x2, Struct);
REFLECT(glm::mat3x3, Struct);
REFLECT(glm::mat3x4, Struct);
DEFINE_REFLECTED_SERIALIZATION(glm::mat3x2);
DEFINE_REFLECTED_SERIALIZATION(glm::mat3x3);
DEFINE_REFLECTED_SERIALIZATION(glm::mat3x4);

REFLECT(glm::mat4x2, Struct);
REFLECT(glm::mat4x3, Struct);
REFLECT(glm::mat4x4, Struct);
DEFINE_REFLECTED_SERIALIZATION(glm::mat4x2);
DEFINE_REFLECTED_SERIALIZATION(glm::mat4x3);
DEFINE_REFLECTED_SERIALIZATION(glm::mat4x4);

