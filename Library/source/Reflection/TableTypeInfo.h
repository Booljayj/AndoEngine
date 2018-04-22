#pragma once
#include <cstddef>
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct TableTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Table;

		TableTypeInfo() = delete;
		TableTypeInfo( void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InInitializer, CLASSIFICATION )
		{}
		virtual ~TableTypeInfo() {}

		TypeInfo* KeyType;
		TypeInfo* ValueType;

		virtual size_t GetCount( void const* Instance ) const = 0;
		virtual void const* FindValue( void const* Instance, void const* Key ) const = 0;
		inline void* FindValue( void* Instance, void const* Key ) const { return const_cast<void*>( FindValue( static_cast<void const*>( Instance ), Key ) ); }

		template<typename TKEY, typename TVALUE, typename TTABLE>
		TVALUE const* FindValue( TTABLE const& Instance, TKEY const& Key ) {
			if(
				TypeResolver<typename std::decay<TKEY>::type>::Get() == KeyType &&
				TypeResolver<typename std::decay<TVALUE>::type>::Get() == ValueType
			) return static_cast<TVALUE const*>( FindValue( &Instance, &Key ) );
			else return nullptr;
		}
		template<typename TKEY, typename TVALUE, typename TTABLE>
		TVALUE* FindValue( TTABLE& Instance, TKEY const& Key ) {
			if(
				TypeResolver<typename std::decay<TKEY>::type>::Get() == KeyType &&
				TypeResolver<typename std::decay<TVALUE>::type>::Get() == ValueType
			) return static_cast<TVALUE*>( FindValue( &Instance, &Key ) );
			else return nullptr;
		}
	};

	template<typename TKEY, typename TVALUE, typename TTABLE>
	struct TTableTypeInfo : public TableTypeInfo
	{
		TTableTypeInfo() = delete;
		TTableTypeInfo( void (*InInitializer)( TypeInfo* ) )
		: TableTypeInfo( InInitializer )
		{}
		virtual ~TTableTypeInfo() {}

		virtual size_t GetCount( void const* Instance ) const override {
			return static_cast<TTABLE const*>( Instance )->size();
		}

		virtual void const* FindValue( void const* Instance, void const* Key ) const override {
			TTABLE const* TableInstance = static_cast<TTABLE const*>( Instance );
			auto Iter = TableInstance->find( *static_cast<TKEY const*>( Key ) );
			if( Iter == TableInstance->end() ) {
				return nullptr;
			} else {
				return &( *Iter );
			}
		}
	};
}
