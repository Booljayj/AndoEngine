#pragma once
#include "Engine/Concepts.h"
#include "Engine/Reflection/ReflectionMacros.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	struct StructTypeInfo;

	namespace Concepts {
		template<typename T>
		concept Class = std::is_class_v<T>;

		template<typename T, typename StructType>
		concept ReflectedStructBase =
			std::is_same_v<T, void> or
			(std::derived_from<StructType, T> and std::is_class_v<T>);
	}

	static const std::initializer_list<VariableInfo const*> no_variables{};

	template<Concepts::Class StructType_>
	struct TStructTypeInfo : public ImplementedTypeInfo<StructType_, StructTypeInfo> {
		using StructType = StructType_;
		using StructTypeInfo::base;
		using ImplementedTypeInfo<StructType, StructTypeInfo>::Cast;

		/** The variables that are contained in this type */
		std::vector<std::unique_ptr<VariableInfo const>> variables;

		TStructTypeInfo(TStructTypeInfo&&) = default;

		template<Concepts::ReflectedStructBase<StructType> BaseType>
		TStructTypeInfo(std::u16string_view name, std::u16string_view description, std::in_place_type_t<BaseType>, std::initializer_list<VariableInfo const*> in_variables)
			: ImplementedTypeInfo<StructType, StructTypeInfo>(::Reflect<StructType>::ID, name, description)
		{
			if constexpr (!std::is_same_v<BaseType, void>) base = &Reflect<BaseType>::Get();

			variables.reserve(in_variables.size());
			for (const auto* var : in_variables) variables.emplace_back(var);
		}

		TStructTypeInfo(std::u16string_view name, std::u16string_view description, std::initializer_list<VariableInfo const*> in_variables)
			: TStructTypeInfo(name, description, std::in_place_type<void>, in_variables)
		{}

		virtual std::span<std::unique_ptr<VariableInfo const> const> GetVariables() const override final {
			return variables;
		}

		virtual void const* GetDefaults() const final {
			if constexpr (std::default_initializable<StructType>) {
				static StructType defaults;
				return &defaults;
			} else {
				return nullptr;
			}
		}

		virtual StructTypeInfo const& GetInstanceTypeInfo(void const* instance) const final {
			if constexpr (Concepts::HasGetTypeInfoMethod<StructType>) return Cast(instance).GetTypeInfo();
			else return *this;
		}
	
	protected:
		virtual void* AllocateRaw() const final {
			if constexpr (std::is_default_constructible_v<StructType>) return new StructType();
			else return nullptr;
		}
	};

	template<typename ValueType>
	struct StaticVariableInfo : public VariableInfo {
		StaticVariableInfo(ValueType* pointer, Hash32 id, std::u16string_view name, std::u16string_view description, FVariableFlags flags)
			: VariableInfo(&Reflect<std::decay_t<ValueType>>::Get(), id, name, description, flags + EVariableFlags::Static), pointer(pointer)
		{}

		virtual void const* GetImmutable(void const* instance) const override final { return pointer; }

		virtual void* GetMutable(void* instance) const override final {
			if constexpr (std::is_const_v<ValueType>) return nullptr;
			else return pointer;
		}

	private:
		ValueType* const pointer;
	};

	template<typename ClassType, typename ValueType>
	struct MemberVariableInfo : public VariableInfo {
		MemberVariableInfo(ValueType ClassType::* pointer, Hash32 id, std::u16string_view name, std::u16string_view description, FVariableFlags flags)
			: VariableInfo(&Reflect<std::decay_t<ValueType>>::Get(), id, name, description, flags), pointer(pointer)
		{}

		virtual void const* GetImmutable(void const* instance) const override final { return &(static_cast<ClassType const*>(instance)->*pointer); }

		virtual void* GetMutable(void* instance) const override final {
			if constexpr (std::is_const_v<ValueType>) return nullptr;
			else return &(static_cast<ClassType*>(instance)->*pointer);
		}

	private:
		ValueType ClassType::* const pointer;
	};

	template<typename ClassType, typename NestedType, typename ValueType>
	struct NestedMemberVariableInfo : public VariableInfo {
		NestedMemberVariableInfo(NestedType ClassType::* container_pointer, ValueType NestedType::* pointer, Hash32 id, std::u16string_view name, std::u16string_view description, FVariableFlags flags)
			: VariableInfo(&Reflect<std::decay_t<ValueType>>::Get(), id, name, description, flags), container_pointer(container_pointer), pointer(pointer)
		{}

		virtual void const* GetImmutable(void const* instance) const override final {
			return &(static_cast<ClassType const*>(instance)->*container_pointer.*pointer);
		}

		virtual void* GetMutable(void* instance) const override final {
			if constexpr (std::is_const_v<ValueType>) return nullptr;
			else return &(static_cast<ClassType*>(instance)->*container_pointer.*pointer);
		}

	private:
		NestedType ClassType::* const container_pointer;
		ValueType NestedType::* const pointer;
	};

	template<typename ClassType, typename ValueType, typename IndexType>
	struct IndexedVariableInfo : public VariableInfo {
		IndexedVariableInfo(size_t index, Hash32 id, std::u16string_view name, std::u16string_view description, FVariableFlags flags)
			: VariableInfo(&Reflect<std::decay_t<ValueType>>::Get(), id, name, description, flags), index(index)
		{}

		virtual void const* GetImmutable(void const* instance) const override final { return &(static_cast<ClassType const*>(instance)->operator[](index)); }

		virtual void* GetMutable(void* instance) const override final {
			if constexpr (std::is_const_v<ValueType>) return nullptr;
			else return &(static_cast<ClassType*>(instance)->operator[](index));
		}

	private:
		IndexType const index;
	};

	namespace StructSerializationHelpers {
		void SerializeVariables(StructTypeInfo const& type, YAML::Node& node, void const* instance);
		void DeserializeVariables(StructTypeInfo const& type, YAML::Node const& node, void* instance);

		void SerializeVariables(StructTypeInfo const& type, Archive::Output& archive, void const* instance);
		void DeserializeVariables(StructTypeInfo const& type, Archive::Input& archive, void* instance);
	}
}

/** Define default archive serialization methods for a struct based on the reflected variables of the struct. */
#define DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(StructType)\
namespace Archive {\
	template<> struct Serializer<StructType> {\
		static inline void Write(Output& archive, StructType const& instance) { ::Reflection::StructSerializationHelpers::SerializeVariables(::Reflect<StructType>::Get(), archive, &instance); }\
		static inline void Read(Input& archive, StructType& instance) { ::Reflection::StructSerializationHelpers::DeserializeVariables(::Reflect<StructType>::Get(), archive, &instance); }\
	};\
}\

/** Define default YAML serialization methods for a struct based on the reflected variables of the struct. */
#define DEFINE_DEFAULT_YAML_SERIALIZATION(StructType)\
namespace YAML {\
	template<> struct convert<StructType> {\
		static inline Node encode(const StructType& instance) {\
			Node node{ NodeType::Map }; ::Reflection::StructSerializationHelpers::SerializeVariables(::Reflect<StructType>::Get(), node, &instance); return node;\
		}\
		static inline bool decode(const Node& node, StructType& instance) {\
			if (!node.IsMap()) return false;\
			::Reflection::StructSerializationHelpers::DeserializeVariables(::Reflect<StructType>::Get(), node, &instance); return true;\
		}\
	};\
}

/** Helper method to create reflection information for a static variable */
template<typename ValueType>
Reflection::VariableInfo const* MakeStatic(ValueType* pointer, Hash32 id, std::u16string_view name, std::u16string_view description, Reflection::FVariableFlags flags = NoFlags) {
	return new Reflection::StaticVariableInfo<ValueType>(pointer, id, name, description, flags);
}

/** Helper method to create reflection information for a non-static member variable */
template<typename ClassType, typename ValueType>
Reflection::VariableInfo const* MakeMember(ValueType ClassType::* pointer, Hash32 id, std::u16string_view name, std::u16string_view description, Reflection::FVariableFlags flags = NoFlags) {
	return new Reflection::MemberVariableInfo<ClassType, ValueType>(pointer, id, name, description, flags);
}

/** Helper method to create reflection information for a non-static member variable nested inside an unnamed struct type */
template<typename ClassType, typename NestedType, typename ValueType>
Reflection::VariableInfo const* MakeNestedMember(NestedType ClassType::* container_pointer, ValueType NestedType::* pointer, Hash32 id, std::u16string_view name, std::u16string_view description, Reflection::FVariableFlags flags = NoFlags) {
	return new Reflection::NestedMemberVariableInfo<ClassType, NestedType, ValueType>(container_pointer, pointer, id, name, description, flags);
}

/** Helper method to create reflection information for a non-static member variable that is accessed using an indexing operator (i.e. for vector or matrix types) */
template<typename ClassType, typename ValueType, typename IndexType>
Reflection::VariableInfo const* MakeIndexed(IndexType index, Hash32 id, std::u16string_view name, std::u16string_view description, Reflection::FVariableFlags flags = NoFlags) {
	return new Reflection::IndexedVariableInfo<ClassType, ValueType, IndexType>(index, id, name, description, flags);
}
