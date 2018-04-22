#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include "Serialization/Serializer.h"

namespace Reflection
{
	enum class ETypeClassification : uint8_t
	{
		Primitive, //Primitive types are fundamental data
		Object, //Objects are types that hold a number of properties of different types
		Sequence, //Containers are types that hold a number of elements of the same type that can be accessed with a number index
		Table, //Tables are types that hold a number of elements of the same type that can be accessed using a key type
		Enumeration, //Enumerations are types which can be a fixed number of predefined values
	};

	/** Flags to describe aspects of a particular type */
	enum class FTypeFlags : uint8_t {
		None = 0,
	};

	struct TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Primitive;

	protected:
		TypeInfo( void (*InInitializer)( TypeInfo* ), ETypeClassification InClassification )
		: Initializer( InInitializer )
		, Classification( InClassification )
		{}

		void (*Initializer)( TypeInfo* );
		ETypeClassification Classification = ETypeClassification::Primitive;
		/** called when loading this type, after the initializer has been executed */
		virtual void OnLoaded( bool bLoadDependencies );

	public:
		TypeInfo() = delete;
		TypeInfo( void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InInitializer, CLASSIFICATION )
		{}
		virtual ~TypeInfo() {}

		//If this is false, none of the following data will be available because the initializer has not run yet.
		bool bIsLoaded = false;

		std::string Name;
		std::string Description;
		size_t Size = 0;

		/** The interface used to serialize this type. If null, this type cannot be serialized. */
		std::unique_ptr<ISerializer> Serializer = nullptr;

		uint16_t NameHash = 0;
		FTypeFlags Flags = FTypeFlags::None;

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
