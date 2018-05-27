#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct DynamicArrayTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::DynamicArray;

		/** The type of the elements in the array */
		TypeInfo const* ElementType = nullptr;

		DynamicArrayTypeInfo() = delete;
		DynamicArrayTypeInfo(
			std::string_view InName, size_t InSize, std::string_view InDescription,
			FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TypeInfo const* InElementType
		);
		virtual ~DynamicArrayTypeInfo() {}

		//Get the number of elements that are in the array
		virtual size_t GetCount( void const* Instance ) const = 0;
		//Get a vector of all the elements in the dynamic array
		virtual void GetElements( std::vector<void*>& OutElements, void* Instance ) const = 0;
		virtual void GetElements( std::vector<void const*>& OutElements, void const* Instance ) const = 0;

		//Resize the array to hold a specific number of elements
		virtual void Resize( void* Instance, size_t Count ) const = 0;

		//Remove all elements in the container
		virtual void ClearElements( void* Instance ) const = 0;
		//Add a new element to the "end" of the array
		virtual void AddElement( void* Instance, void const* Value ) const = 0;

		//Remove the element that equals the pointer
		virtual void RemoveElement( void* Instance, void const* ElementPointer ) const = 0;
		//Insert a new element at the position of the element that equals the pointer, equal to the value
		virtual void InsertElement( void* Instance, void const* ElementPointer, void const* Value ) const = 0;
	};

	template<typename TELEMENT, typename TARRAY>
	struct TDynamicArrayTypeInfo : public DynamicArrayTypeInfo
	{
		TDynamicArrayTypeInfo() = delete;
		TDynamicArrayTypeInfo( std::string_view InDescription, FTypeFlags InFlags )
		: DynamicArrayTypeInfo(
			TypeResolver<TARRAY>::GetName(), sizeof( TARRAY ), InDescription,
			InFlags, nullptr,
			TypeResolver<TELEMENT>::Get()
		)
		{}
		virtual ~TDynamicArrayTypeInfo() {}

		static TARRAY const& Cast( void const* Instance ) { return *static_cast<TARRAY const*>( Instance ); }
		static TARRAY& Cast( void* Instance ) { return *static_cast<TARRAY*>( Instance ); }
		static TELEMENT const& CastElement( void const* Element ) { return *static_cast<TELEMENT const*>( Element ); }

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

		virtual void Resize( void* Instance, size_t Count ) const override { Cast( Instance ).resize( Count ); }

		virtual void ClearElements( void* Instance ) const override { Cast( Instance ).clear(); }
		virtual void AddElement( void* Instance, void const* Value ) const override { Cast( Instance ).push_back( CastElement( Value ) ); }

		virtual void RemoveElement( void* Instance, void const* ElementPointer ) const override {
			typename TARRAY::iterator Position = std::find_if(
				Cast( Instance ).begin(), Cast( Instance ).end(),
				[=]( TELEMENT const& Element ){ return &Element == ElementPointer; }
			);
			Cast( Instance ).erase( Position );
		}
		virtual void InsertElement( void* Instance, void const* ElementPointer, void const* Value ) const override {
			typename TARRAY::iterator Position = std::find_if(
				Cast( Instance ).begin(), Cast( Instance ).end(),
				[=]( TELEMENT const& Element ){ return &Element == ElementPointer; }
			);
			Cast( Instance ).insert( Position, CastElement( Value ) );
		}
	};
}
