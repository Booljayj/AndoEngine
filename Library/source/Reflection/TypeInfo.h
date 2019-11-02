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

	/** Flags to describe aspects of a particular type */
	enum class FTypeFlags : uint8_t {
		None = 0,
		//This type can be serialized to disk (binary or text)
		Serializable = 1 << 0,
		//This type can be serialized over the network (fast binary)
		NetSerializable = 1 << 1,
	};
	DEFINE_BITFLAG_OPERATORS(FTypeFlags);

	/** Provides a set of runtime information about a type */
	struct TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Primitive;

	public:
		//============================================================
		// Basic required type information

		/** The classification of this TypeInfo, defining what kinds of type information it contains */
		ETypeClassification Classification = ETypeClassification::Primitive;
		/** The identifier for this type. Always unique and stable. */
		Hash128 UniqueID = Hash128{};
		/** Definitions for this type created by the compiler */
		CompilerDefinition Definition;

		//============================================================
		// Optional type information

		/** Human-readable description of this type */
		std::string_view Description = std::string_view{};
		/** Flags that provide additional information about this type */
		FTypeFlags Flags = FTypeFlags::None;
		/** The interface used to serialize this type */
		Serialization::ISerializer* Serializer = nullptr;

		/** Get the container that includes all TypeInfo objects that exist. */
		static std::deque<TypeInfo const*> const& GetGlobalTypeInfoCollection();

		/** Find a TypeInfo object using its unique ID */
		static TypeInfo const* FindTypeByID(Hash128 UniqueID);

		TypeInfo() = delete;
		TypeInfo(
			ETypeClassification InClassification, Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer
		);
		virtual ~TypeInfo() = default;

		/** Construct an instance of this type at the address using the default constructor */
		virtual void Construct(void* Instance) const = 0;
		/** Construct an instance of this type at the address using the copy constructor and an existing instance */
		virtual void Construct(void* Instance, void const* Template) const = 0;
		/** Destruct an instance of this type at the address. Assumes the instance was properly constructed and won't be destructed again */
		virtual void Destruct(void* Instance) const = 0;
		/** Compare two instances of this type and return true if they should be considered equal */
		virtual bool Equal(void const* InstanceA, void const* InstanceB) const = 0;

		/** Convert this TypeInfo to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
		template<typename TypeInfoType>
		TypeInfoType const* As() const {
			if (TypeInfoType::CLASSIFICATION == Classification) return static_cast<TypeInfoType const*>(this);
			else return nullptr;
		}
	};

	/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
	template<typename TypeInfoType>
	TypeInfoType const* Cast(TypeInfo const* Info) {
		if (!Info) return nullptr;
		else return Info->As<TypeInfoType>();
	}
}

#define STANDARD_TYPEINFO_METHODS(Type)\
static constexpr Type const& Cast(void const* P) { return *static_cast<Type const*>(P); }\
static constexpr Type& Cast(void* P) { return *static_cast<Type*>(P); }\
virtual void Construct(void* I) const final {new (I) Type;}\
virtual void Construct(void* I, void const* T) const final { new (I) Type(Cast(T));}\
virtual void Destruct(void* I) const final {Cast(I).~Type();}\
virtual bool Equal(void const* A, void const* B) const final {return Cast(A) == Cast(B);}
