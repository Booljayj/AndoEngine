#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
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

	struct TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Primitive;

		TypeInfo() = delete;
		TypeInfo( void (*Initializer)( TypeInfo* ), std::string&& InName, size_t InSize );
		TypeInfo( ETypeClassification InClassification, std::string&& InName, size_t InSize );
		virtual ~TypeInfo() {}

		//============================================================
		// Basic required type information
		/** The classification of this TypeInfo, defining what kinds of type information it contains */
		ETypeClassification Classification = ETypeClassification::Primitive;
		/** The fully qualified name of the type, including namespaces. If this is a template, does not include template arguments */
		std::string Name;
		/** The hash of the fully qualified name, serves as a unique identifier */
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

		/** Get the full name of this type, including template arguments if it is a template */
		virtual std::string_view GetName() const { return Name; }
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
