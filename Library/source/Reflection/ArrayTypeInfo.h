#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include "Reflection/BaseResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct ArrayTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Array;

		/** Whether the number of elements in the array can be manipulated */
		bool IsFixedSize = false;

		/** The type of the elements in the array */
		TypeInfo const* ElementType = nullptr;

		ArrayTypeInfo() = delete;
		ArrayTypeInfo( std::string_view InName, size_t InSize, std::string_view InDescription, bool InIsFixedSize, TypeInfo const* InElementType );
		virtual ~ArrayTypeInfo() {}

		//Get the number of elements that are in the array
		virtual size_t GetCount( void const* Instance ) const = 0;

		//Get a vector of all the elements in the dynamic array
		virtual void GetElements( void* Instance, std::vector<void*>& OutElements ) const = 0;
		virtual void GetElements( void const* Instance, std::vector<void const*>& OutElements ) const = 0;

		//Resize the array to hold a specific number of elements
		virtual void Resize( void* Instance, size_t Count ) const = 0;

		//Remove all elements in the container
		virtual void ClearElements( void* Instance ) const = 0;
		//Add a new element to the "end" of the array
		virtual void AddElement( void* Instance, void const* Value ) const = 0;

		//Remove the pointed-at element
		virtual void RemoveElement( void* Instance, void const* ElementPointer ) const = 0;
		//Insert a new element at the position of the pointed-at element, equal to the value. If value is nullptr, the new element is default-constructed
		virtual void InsertElement( void* Instance, void const* ElementPointer, void const* Value ) const = 0;
	};

	/** Template that implements the ArrayTypeInfo interface for fixed-size arrays (std::array) */
	template<typename TELEMENT, size_t SIZE, typename TARRAY>
	struct TFixedArrayTypeInfo : public ArrayTypeInfo
	{
		TFixedArrayTypeInfo() = delete;
		TFixedArrayTypeInfo( std::string_view InDescription )
		: ArrayTypeInfo( TypeResolver<TARRAY>::GetName(), sizeof( TARRAY ), InDescription, true, TypeResolver<TELEMENT>::Get() )
		{}

		static constexpr TARRAY const& Cast( void const* Instance ) { return *static_cast<TARRAY const*>( Instance ); }
		static constexpr TARRAY& Cast( void* Instance ) { return *static_cast<TARRAY*>( Instance ); }

		virtual size_t GetCount( void const* Instance ) const override { return SIZE; }

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

		virtual void Resize( void* Instance, size_t Count ) const override {}
		virtual void ClearElements( void* Instance ) const override {}
		virtual void AddElement( void* Instance, void const* Value ) const override {}
		virtual void RemoveElement( void* Instance, void const* ElementPointer ) const override {}
		virtual void InsertElement( void* Instance, void const* ElementPointer, void const* Value ) const override {}
	};

	/** Template that implements the ArrayTypeInfo interface for dynamic array types (std::vector, std::list, etc) */
	template<typename TELEMENT, typename TARRAY>
	struct TDynamicArrayTypeInfo : public ArrayTypeInfo
	{
		TDynamicArrayTypeInfo() = delete;
		TDynamicArrayTypeInfo( std::string_view InDescription )
		: ArrayTypeInfo( TypeResolver<TARRAY>::GetName(), sizeof( TARRAY ), InDescription, false, TypeResolver<TELEMENT>::Get() )
		{}
		virtual ~TDynamicArrayTypeInfo() {}

		static constexpr TARRAY const& Cast( void const* Instance ) { return *static_cast<TARRAY const*>( Instance ); }
		static constexpr TARRAY& Cast( void* Instance ) { return *static_cast<TARRAY*>( Instance ); }
		static constexpr TELEMENT const& CastElement( void const* Element ) { return *static_cast<TELEMENT const*>( Element ); }

		virtual size_t GetCount( void const* Instance ) const override {
			return Cast( Instance ).size();
		}

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

		virtual void Resize( void* Instance, size_t Count ) const override {
			Cast( Instance ).resize( Count );
		}

		virtual void ClearElements( void* Instance ) const override {
			Cast( Instance ).clear();
		}
		virtual void AddElement( void* Instance, void const* Value ) const override {
			Cast( Instance ).push_back( CastElement( Value ) );
		}

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
			if( Value ) {
				Cast( Instance ).insert( Position, CastElement( Value ) );
			} else {
				Cast( Instance ).insert( Position, TELEMENT{} );
			}
		}
	};
}
