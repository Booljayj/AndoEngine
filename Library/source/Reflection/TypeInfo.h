#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <deque>
#include <ostream>
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Serialization/Serializer.h"
#include "Reflection/CompilerDefinition.h"

namespace Reflection
{
	enum class ETypeClassification : uint8_t {
		//Basic construct types
		Primitive,
		Struct,
		Alias,
		Enumeration,
		Flags,
		//Homogeneous collection types
		Array,
		Map,
		Set,
		//Heterogeneous collection types
		Poly,
		Tuple,
		Variant,
	};

	/** flags to describe aspects of a particular type */
	enum class ETypeFlags : uint8_t {
		//This type can be serialized to disk (binary or text)
		Serializable,
		//This type can be serialized over the network (fast binary)
		NetSerializable,
	};
	using FTypeFlags = TFlags<ETypeFlags>;

	/** Provides a set of runtime information about a type */
	struct TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Primitive;

	public:
		//============================================================
		// Basic required type information

		/** The classification of this TypeInfo, defining what kinds of type information it contains */
		ETypeClassification classification = ETypeClassification::Primitive;
		/** The identifier for this type. Always unique and stable. */
		Hash128 id = Hash128{};
		/** Definitions for this type created by the compiler */
		CompilerDefinition def;

		//============================================================
		// Optional type information

		/** Human-readable description of this type */
		std::string_view description = std::string_view{};
		/** Flags that provide additional information about this type */
		FTypeFlags flags = FTypeFlags::None;
		/** The interface used to serialize this type */
		Serialization::ISerializer* serializer = nullptr;

		/** Get the container that includes all TypeInfo objects that exist. */
		static std::deque<TypeInfo const*> const& GetGlobalTypeInfoCollection();

		/** Find a TypeInfo object using its unique ID */
		static TypeInfo const* FindTypeByID(Hash128 id);

		TypeInfo() = delete;
		TypeInfo(
			ETypeClassification inClassification, Hash128 inUniqueID, CompilerDefinition inDefinition,
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer
		);
		virtual ~TypeInfo() = default;

		/** Construct an instance of this type at the address using the default constructor */
		virtual void Construct(void* instance) const = 0;
		/** Construct an instance of this type at the address using the copy constructor and an existing instance */
		virtual void Construct(void* instance, void const* other) const = 0;
		/** Destruct an instance of this type at the address. Assumes the instance was properly constructed and won't be destructed again */
		virtual void Destruct(void* instance) const = 0;
		/** Compare two instances of this type and return true if they should be considered equal */
		virtual bool Equal(void const* a, void const* b) const = 0;

		/** Convert this TypeInfo to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
		template<typename TypeInfoType>
		TypeInfoType const* As() const {
			static_assert(std::is_base_of_v<TypeInfo, TypeInfoType>, "TypeInfo::As may only convert to types that derive from TypeInfo");
			if (TypeInfoType::Classification == classification) return static_cast<TypeInfoType const*>(this);
			else return nullptr;
		}
	};

	/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
	template<typename TypeInfoType>
	TypeInfoType const* Cast(TypeInfo const* info) {
			static_assert(std::is_base_of_v<TypeInfo, TypeInfoType>, "TypeInfo::Cast may only convert to types that derive from TypeInfo");
		if (!info) return nullptr;
		else return info->As<TypeInfoType>();
	}
}

#define STANDARD_TYPEINFO_METHODS(Type)\
static constexpr Type const& Cast(void const* pointer) { return *static_cast<Type const*>(pointer); }\
static constexpr Type& Cast(void* pointer) { return *static_cast<Type*>(pointer); }\
virtual void Construct(void* instance) const final {new (instance) Type;}\
virtual void Construct(void* instance, void const* other) const final { new (instance) Type(Cast(other));}\
virtual void Destruct(void* instance) const final {Cast(instance).~Type();}\
virtual bool Equal(void const* a, void const* b) const final {return Cast(a) == Cast(b);}
