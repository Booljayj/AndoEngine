#pragma once
#include <cstddef>
#include "Reflection/Resolver/TypeResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct MapTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Map;

		MapTypeInfo() = delete;
		MapTypeInfo( std::string_view InName, size_t InSize );
		virtual ~MapTypeInfo() {}

		TypeInfo const* KeyType = nullptr;
		TypeInfo const* ValueType = nullptr;

		virtual size_t GetCount( void const* Instance ) const = 0;
	};

	template<typename TKEY, typename TVALUE, typename TTABLE>
	struct TMapTypeInfo : public MapTypeInfo
	{
		TMapTypeInfo() = delete;
		TMapTypeInfo( void (*Initializer)( MapTypeInfo* ) )
		: MapTypeInfo( TypeResolver<TTABLE>::GetName(), sizeof( TTABLE ) )
		{
			if( Initializer ) Initializer( this );
		}

		static TTABLE const& Cast( void const* Instance ) { return *static_cast<TTABLE const*>( Instance ); }
		static TTABLE& Cast( void* Instance ) { return *static_cast<TTABLE*>( Instance ); }

		virtual size_t GetCount( void const* Instance ) const override {
			return Cast( Instance ).size();
		}
	};
}
