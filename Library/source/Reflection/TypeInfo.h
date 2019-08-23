#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <deque>
#include <ostream>
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
	};

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
		char const* Description = nullptr;
		/** Flags that provide additional information about this type */
		FTypeFlags Flags = FTypeFlags::None;
		/** The interface used to serialize this type. If null, this type cannot be serialized. */
		Serialization::ISerializer* Serializer = nullptr;

		/** Get the container that includes all TypeInfo objects that exist. */
		static std::deque<TypeInfo const*> const& GetGlobalTypeInfoCollection();

		/** Find a TypeInfo object using its unique ID */
		static TypeInfo const* FindTypeByID( Hash128 UniqueID );

		TypeInfo() = delete;
		TypeInfo(
			ETypeClassification InClassification, Hash128 InUniqueID, CompilerDefinition InDefinition,
			char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer
		);
		virtual ~TypeInfo() = default;

		/** Construct an instance of this type at the address using the default constructor. Assumes enough space has been allocated to fit this type */
		virtual void Construct( void* A ) const = 0;
		/** Destruct an instance of this type at the address. Assumes the instance was properly constructed and won't be destructed again */
		virtual void Destruct( void* A ) const = 0;
		/** Copy one instance of this type into another. The destination instance must be initialized. Most types will deep copy, but some may shallow copy if appropriate. */
		virtual void Copy(void* A, void const* B) const {}
		/** Compare two instances of this type and return true if they should be considered equal */
		virtual bool Equal( void const* A, void const* B ) const = 0;

		/** Convert this TypeInfo to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
		template<typename TTYPE>
		TTYPE const* As() const {
			if( TTYPE::CLASSIFICATION == Classification ) return static_cast<TTYPE const*>( this );
			else return nullptr;
		}
	};

	/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
	template<typename TTYPE>
	TTYPE const* Cast( TypeInfo const* Info ) {
		if( !Info ) return nullptr;
		else return Info->As<TTYPE>();
	}
}

#define STANDARD_TYPEINFO_METHODS(Type)\
static constexpr Type const& Cast(void const* P) { return *static_cast<Type const*>(P); }\
static constexpr Type& Cast(void* P) { return *static_cast<Type*>(P); }\
virtual void Construct(void* P) const final {new (P) Type;}\
virtual void Destruct(void* P) const final {Cast(P).~Type();}\
virtual void Copy(void* A, void const* B) const final {Cast(A) = Cast(B);}\
virtual bool Equal(void const* A, void const* B) const final {return Cast(A) == Cast(B);}
