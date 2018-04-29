#pragma once
#include <cstddef>
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct MapTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Map;

		MapTypeInfo() = delete;
		MapTypeInfo( char const* InName, size_t InSize, void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InName, InSize, InInitializer, CLASSIFICATION )
		{}
		virtual ~MapTypeInfo() {}

		TypeInfo* KeyType = nullptr;
		TypeInfo* ValueType = nullptr;

		virtual void OnLoaded( bool bLoadDependencies ) override;

		virtual size_t GetCount( void const* Instance ) const = 0;
	};

	template<typename TKEY, typename TVALUE, typename TTABLE>
	struct TMapTypeInfo : public MapTypeInfo
	{
		TMapTypeInfo() = delete;
		TMapTypeInfo( char const* InName, size_t InSize, void (*InInitializer)( TypeInfo* ) )
		: MapTypeInfo( InName, InSize, InInitializer )
		{}
		virtual ~TMapTypeInfo() {}

		static TTABLE const& Cast( void const* Instance ) { return *static_cast<TTABLE const*>( Instance ); }
		static TTABLE& Cast( void* Instance ) { return *static_cast<TTABLE*>( Instance ); }

		virtual size_t GetCount( void const* Instance ) const override {
			return Cast( Instance ).size();
		}
	};
}
