#pragma once
#include <cstdint>
#include <string>
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

	protected:
		TypeInfo( char const* InName, size_t InSize, void (*InInitializer)( TypeInfo* ), ETypeClassification InClassification )
		: Initializer( InInitializer )
		, Classification( InClassification )
		, Name( InName )
		, Size( InSize )
		{}

		void (*Initializer)( TypeInfo* );
		ETypeClassification Classification = ETypeClassification::Primitive;
		/** called when loading this type, after the initializer has been executed */
		virtual void OnLoaded( bool bLoadDependencies );

	public:
		TypeInfo() = delete;
		TypeInfo( char const* InName, size_t InSize, void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InName, InSize, InInitializer, CLASSIFICATION )
		{}
		virtual ~TypeInfo() {}

		//If this is false, none of the following data will be available because the initializer has not run yet.
		bool bIsLoaded = false;

		char const* Name;
		size_t Size = 0;
		uint16_t NameHash = 0;

		std::string Description;
		FTypeFlags Flags = FTypeFlags::None;

		/** The interface used to serialize this type. If null, this type cannot be serialized. */
		std::unique_ptr<ISerializer> Serializer = nullptr;

		/** Load all the data for this type */
		void Load( bool bLoadDependencies = true );

		/** Get a pointer to a specific kind of type. Will return nullptr if the conversion is not possible */
		template<typename TDATA>
		TDATA const* As() const {
			if( TDATA::CLASSIFICATION == Classification ) return (TDATA const*)this;
			else return nullptr;
		}
		template<typename TDATA>
		TDATA* As() { return const_cast<TDATA*>( static_cast<TypeInfo const*>( this )->As<TDATA>() ); }
	};
}
