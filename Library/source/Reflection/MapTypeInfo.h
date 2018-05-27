#pragma once
#include <cstddef>
#include "Reflection/Resolver/TypeResolver.h"
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
		MapTypeInfo(
			std::string_view InName, size_t InSize, std::string_view InDescription,
			FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TypeInfo const* InKeyType, TypeInfo const* InValueType
		);
		virtual ~MapTypeInfo() {}

		virtual size_t GetCount( void const* Instance ) const = 0;
	};

	template<typename TKEY, typename TVALUE, typename TMAP>
	struct TMapTypeInfo : public MapTypeInfo
	{
		TMapTypeInfo() = delete;
		TMapTypeInfo( std::string_view InDescription, FTypeFlags InFlags )
		: MapTypeInfo(
			TypeResolver<TMAP>::GetName(), sizeof( TMAP ), InDescription,
			InFlags, nullptr,
			TypeResolver<TKEY>::Get(), TypeResolver<TVALUE>::Get()
		)
		{}

		static TMAP const& Cast( void const* Instance ) { return *static_cast<TMAP const*>( Instance ); }
		static TMAP& Cast( void* Instance ) { return *static_cast<TMAP*>( Instance ); }

		virtual size_t GetCount( void const* Instance ) const override {
			return Cast( Instance ).size();
		}
	};
}
