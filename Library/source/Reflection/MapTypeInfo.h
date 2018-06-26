#pragma once
#include <cstddef>
#include <utility>
#include <vector>
#include "Reflection/Resolver/BaseResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct MapTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Map;

		/** The type of the keys in the map */
		TypeInfo const* KeyType = nullptr;
		/** The type of the values in the map */
		TypeInfo const* ValueType = nullptr;

		MapTypeInfo() = delete;
		MapTypeInfo( std::string_view InName, size_t InSize, std::string_view InDescription, TypeInfo const* InKeyType, TypeInfo const* InValueType );
		virtual ~MapTypeInfo() {}

		// Get the number of entries in this map
		virtual size_t GetCount( void const* Instance ) const = 0;

		// Get a vector of all entry pairs in this map
		virtual void GetPairs( void* Instance, std::vector<std::pair<void const*, void*>> OutPairs ) const = 0;
		virtual void GetPairs( void const* Instance, std::vector<std::pair<void const*, void const*>> OutPairs ) const = 0;

		// Find the value for a key. Returns nullptr if the key is not in the map
		virtual void* Find( void* Instance, void const* Key ) const = 0;
		virtual void const* Find( void const* Instance, void const* Key ) const = 0;

		// Remove all entries from the map
		virtual void Clear( void* Instance ) const = 0;

		// Remove the entry corresponding to a particular key from the map. Returns true if successful.
		virtual bool RemoveEntry( void* Instance, void const* Key ) const = 0;
		// Add a new entry to the map with the specified value. If the key is already in the map, nothing will happen and will return false. If Value is nullptr, the new value will be default constructed
		virtual bool InsertEntry( void* Instance, void const* Key, void const* Value ) const = 0;
	};

	template<typename TKEY, typename TVALUE, typename TMAP>
	struct TMapTypeInfo : public MapTypeInfo
	{
		TMapTypeInfo() = delete;
		TMapTypeInfo( std::string_view InDescription )
		: MapTypeInfo( TypeResolver<TMAP>::GetName(), sizeof( TMAP ), InDescription, TypeResolver<TKEY>::Get(), TypeResolver<TVALUE>::Get() )
		{}

		static constexpr TMAP const& Cast( void const* Instance ) { return *static_cast<TMAP const*>( Instance ); }
		static constexpr TMAP& Cast( void* Instance ) { return *static_cast<TMAP*>( Instance ); }
		static constexpr TKEY const& CastKey( void const* Key ) { return *static_cast<TKEY const*>( Key ); }
		static constexpr TVALUE const& CastValue( void const* Value ) { return *static_cast<TVALUE const*>( Value ); }

		virtual size_t GetCount( void const* Instance ) const override {
			return Cast( Instance ).size();
		}

		virtual void GetPairs( void* Instance, std::vector<std::pair<void const*, void*>> OutPairs ) const override {
			OutPairs.clear();
			for( auto& Iter : Cast( Instance ) ) {
				OutPairs.push_back( std::make_pair( &Iter.first, &Iter.second ) );
			}
		}
		virtual void GetPairs( void const* Instance, std::vector<std::pair<void const*, void const*>> OutPairs ) const override {
			OutPairs.clear();
			for( auto const& Iter : Cast( Instance ) ) {
				OutPairs.push_back( std::make_pair( &Iter.first, &Iter.second ) );
			}
		}

		virtual void* Find( void* Instance, void const* Key ) const override {
			auto const Iter = Cast( Instance ).find( CastKey( Key ) );
			if( Iter != Cast( Instance ).end() ) {
				return &( *Iter );
			} else {
				return nullptr;
			}
		}
		virtual void const* Find( void const* Instance, void const* Key ) const override {
			auto const Iter = Cast( Instance ).find( CastKey( Key ) );
			if( Iter != Cast( Instance ).end() ) {
				return &( *Iter );
			} else {
				return nullptr;
			}
		}

		virtual void Clear( void* Instance ) const override {
			Cast( Instance ).clear();
		}

		virtual bool RemoveEntry( void* Instance, void const* Key ) const override {
			size_t RemovedCount = Cast( Instance ).erase( CastKey( Key ) );
			return RemovedCount > 0;
		}
		virtual bool InsertEntry( void* Instance, void const* Key, void const* Value ) const override {
			if( Value ) {
				return Cast( Instance ).insert( std::make_pair( CastKey( Key ), CastValue( Value ) ) ).second;
			} else {
				return Cast( Instance ).insert( std::make_pair( CastKey( Key ), TVALUE{} ) ).second;
			}
		}
	};
}
