#pragma once
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"
#include "Serialization/FixedArraySerializer.h"

namespace Reflection {
	struct FixedArrayTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::FixedArray;

		/** The type of the elements in the array */
		TypeInfo const* ElementType = nullptr;
		/** The number of elements in the array */
		size_t Count = 0;

		FixedArrayTypeInfo() = delete;
		FixedArrayTypeInfo( std::string_view InName, size_t InSize, std::string_view InDescription, TypeInfo const* InElementType, size_t InCount );
		virtual ~FixedArrayTypeInfo() {}

		//Get a vector of all the elements in the array
		virtual void GetElements( void* Instance, std::vector<void*>& OutElements ) const = 0;
		virtual void GetElements( void const* Instance, std::vector<void const*>& OutElements ) const = 0;
	};

	template<typename TELEMENT, size_t SIZE, typename TARRAY>
	struct TFixedArrayTypeInfo : public FixedArrayTypeInfo
	{
		TFixedArrayTypeInfo() = delete;
		TFixedArrayTypeInfo( std::string_view InDescription )
		: FixedArrayTypeInfo( TypeResolver<TARRAY>::GetName(), sizeof( TARRAY ), InDescription, TypeResolver<TELEMENT>::Get(), SIZE )
		{}

		static constexpr TARRAY const& Cast( void const* Instance ) { return *static_cast<TARRAY const*>( Instance ); }
		static constexpr TARRAY& Cast( void* Instance ) { return *static_cast<TARRAY*>( Instance ); }

		virtual void GetElements( void* Instance, std::vector<void*>& OutElements ) const override {
			OutElements.clear();
			for( TELEMENT& ArrayElement : Cast( Instance ) ) {
				OutElements.push_back( &ArrayElement );
			}
		}
		virtual void GetElements( void const* Instance, std::vector<void const*>& OutElements ) const override {
			OutElements.clear();
			for( TELEMENT const& ArrayElement : Cast( Instance ) ) {
				OutElements.push_back( &ArrayElement );
			}
		}
	};
}
