#pragma once
#include <cstddef>
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct MapTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Map;

		MapTypeInfo() = delete;
		MapTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: TypeInfo( CLASSIFICATION, InInitializer, std::forward<std::string>( InName ), InSize )
		{}
		virtual ~MapTypeInfo() {}

		TypeInfo* KeyType = nullptr;
		TypeInfo* ValueType = nullptr;

		virtual size_t GetCount( void const* Instance ) const = 0;
	};

	template<typename TKEY, typename TVALUE, typename TTABLE>
	struct TMapTypeInfo : public MapTypeInfo
	{
		TMapTypeInfo() = delete;
		TMapTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize )
		: MapTypeInfo( InInitializer, std::forward<std::string>( InName ), InSize )
		{}
		virtual ~TMapTypeInfo() {}

		static TTABLE const& Cast( void const* Instance ) { return *static_cast<TTABLE const*>( Instance ); }
		static TTABLE& Cast( void* Instance ) { return *static_cast<TTABLE*>( Instance ); }

		virtual size_t GetCount( void const* Instance ) const override {
			return Cast( Instance ).size();
		}
	};
}
