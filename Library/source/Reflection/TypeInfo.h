#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <deque>
#include <ostream>
#include "Serialization/Serializer.h"

namespace Reflection
{
	enum class ETypeClassification : uint8_t {
		//Basic construct types
		Primitive,
		Struct,
		Enumeration,
		//Heterogeneous collection types
		Tuple,
		Variant,
		//Homogeneous collection types
		FixedArray,
		DynamicArray,
		Map,
		Set,
	};

	/** Flags to describe aspects of a particular type */
	enum class FTypeFlags : uint8_t {
		None = 0,
	};

	/** Provides a set of runtime information about a type */
	struct TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Primitive;
		/** The global list of all TypeInfo objects that have been created */
		static std::deque<TypeInfo const*> GlobalTypeCollection;

		//============================================================
		// Basic required type information
		/** The classification of this TypeInfo, defining what kinds of type information it contains */
		ETypeClassification Classification = ETypeClassification::Primitive;
		/** The human-readable name of this type */
		std::string Name;
		/** The hash of the name, serves as a unique identifier */
		uint32_t NameHash = 0;
		/** The size in bytes of an instance of this type */
		size_t Size = 0;

		//============================================================
		// Optional type information
		/** Human-readable description of this type */
		std::string Description;
		/** Flags that provide additional information about this type */
		FTypeFlags Flags = FTypeFlags::None;
		/** The interface used to serialize this type. If null, this type cannot be serialized. */
		std::unique_ptr<Serialization::ISerializer> Serializer = nullptr;

		/** Print a description of a TypeInfo to a stream */
		static void Print( TypeInfo const* Info, std::ostream& Stream );
		/** Print basic information about all TypeInfos to a stream */
		static void PrintAll( std::ostream& Stream );

		/** Find a TypeInfo object using the hash of its name */
		static TypeInfo const* FindTypeByNameHash( uint32_t NameHash );
		/** Find a TypeInfo object using its name */
		static TypeInfo const* FindTypeByName( std::string_view Name );

		TypeInfo() = delete;
		TypeInfo( ETypeClassification InClassification, std::string_view InName, size_t InSize, std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer );
		TypeInfo( ETypeClassification InClassification, std::string_view InName, size_t InSize );
		virtual ~TypeInfo() {}

		/** Compare two instances of this type, similar to standard compare functions */
		virtual int8_t Compare( void const*, void const* ) const;

		/** Get a pointer to a specific kind of type. Will return nullptr if the conversion is not possible */
		template<typename TTYPE>
		TTYPE const* As() const {
			if( TTYPE::CLASSIFICATION == Classification ) return (TTYPE const*)this;
			else return nullptr;
		}
		template<typename TTYPE>
		TTYPE* As() { return const_cast<TTYPE*>( static_cast<TypeInfo const*>( this )->As<TTYPE>() ); }
	};
}
