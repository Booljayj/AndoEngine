#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include "Serialization/Serializer.h"

#define REFLECT()\
static Reflection::TypeInfo* StaticGetTypeInfo();\
virtual Reflection::TypeInfo* GetTypeInfo() const

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
	public:
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Primitive;

		TypeInfo() = delete;
		TypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize );
		virtual ~TypeInfo() {}

	protected:
		TypeInfo( ETypeClassification InClassification, void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize );

		//TypeInfo required setup data
		ETypeClassification Classification = ETypeClassification::Primitive;
		void (*Initializer)( TypeInfo* ) = nullptr;

		//Basic type information
		std::string Name;
		uint32_t NameHash = 0;
		size_t Size = 0;

		//Flag that tracks if this type was already loaded
		bool bIsLoaded = false;

	public:
		/** Human-readable description of this type */
		std::string Description;
		/** Flags that provide additional information about this type */
		FTypeFlags Flags = FTypeFlags::None;
		/** The interface used to serialize this type. If null, this type cannot be serialized. */
		std::unique_ptr<Serialization::ISerializer> Serializer = nullptr;

		/** The function used to compare two instances of this type */
		int8_t (*Compare)( TypeInfo*, void const*, void const* ) = nullptr;

		/** Default comparison function that compares memory contents */
		static int8_t DefaultCompare( TypeInfo* Info, void const* A, void const* B );

		/** Load all the data for this type, allowing it to be fully used. */
		void Load();

		/** Get the name of this type */
		inline std::string_view GetName() const { return Name; }
		/** Get the unique identifier for this type */
		inline uint16_t GetNameHash() const { return NameHash; }
		/** Get the size in bytes of this type */
		inline size_t GetSize() const { return Size; }

		/** Get a pointer to a specific kind of type. Will return nullptr if the conversion is not possible */
		template<typename TTYPE>
		TTYPE const* As() const {
			if( TTYPE::CLASSIFICATION == Classification ) return (TTYPE const*)this;
			else return nullptr;
		}
		template<typename TTYPE>
		TTYPE* As() { return const_cast<TTYPE*>( static_cast<TypeInfo const*>( this )->As<TTYPE>() ); }

	protected:
		/** called when loading this type, after the initializer has been executed */
		virtual void OnLoaded();
	};
}
