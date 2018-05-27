#pragma once
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct FixedArrayTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::FixedArray;

		TypeInfo const* ElementType = nullptr;

		FixedArrayTypeInfo() = delete;
		FixedArrayTypeInfo( std::string_view InName, size_t InSize );
		virtual ~FixedArrayTypeInfo() {}

		//Get the number of elements that are in the array
		virtual size_t GetCount( void const* Instance ) const = 0;
		//Get a vector of all the elements in the array
		virtual void GetElements( std::vector<void*>& OutElements, void* Instance ) const = 0;
		virtual void GetElements( std::vector<void const*>& OutElements, void const* Instance ) const = 0;
	};

	template<typename TELEMENT, typename TARRAY>
	struct TFixedArrayTypeInfo : public FixedArrayTypeInfo
	{
		TFixedArrayTypeInfo() = delete;
		TFixedArrayTypeInfo( void (*Initializer)( FixedArrayTypeInfo* ) )
		: FixedArrayTypeInfo( TypeResolver<TARRAY>::GetName(), sizeof( TARRAY ) )
		{
			if( Initializer ) Initializer( this );
		}

		static TARRAY const& Cast( void const* Instance ) { return *static_cast<TARRAY const*>( Instance ); }
		static TARRAY& Cast( void* Instance ) { return *static_cast<TARRAY*>( Instance ); }

		virtual size_t GetCount( void const* Instance ) const override {
			return Cast( Instance ).size();
		}
		virtual void GetElements( std::vector<void*>& OutElements, void* Instance ) const override {
			OutElements.clear();
			for( TELEMENT& ArrayElement : Cast( Instance ) ) {
				OutElements.push_back( &ArrayElement );
			}
		}
		virtual void GetElements( std::vector<void const*>& OutElements, void const* Instance ) const override {
			OutElements.clear();
			for( TELEMENT const& ArrayElement : Cast( Instance ) ) {
				OutElements.push_back( &ArrayElement );
			}
		}
	};
}
