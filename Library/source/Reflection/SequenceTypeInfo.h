#pragma once
#include <cstddef>
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct SequenceTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Sequence;

		SequenceTypeInfo() = delete;
		SequenceTypeInfo( void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InInitializer, CLASSIFICATION )
		{}
		virtual ~SequenceTypeInfo() {}

		TypeInfo* ElementType;

		virtual size_t GetCount( void const* Instance ) const = 0;
		virtual void const* FindElement( void const* Instance, size_t Index ) const = 0;
		inline void* FindElement( void* Instance, size_t Index ) const { return const_cast<void*>( FindElement( static_cast<void const*>( Instance ), Index ) ); }

		template<typename TELEMENT, typename TSEQUENCE>
		TELEMENT const* FindElement( TSEQUENCE const& Instance, size_t Index ) const {
			if( TypeResolver<TELEMENT>::Get() == ElementType ) return (TELEMENT const*)FindElement( &Instance, Index );
			else return nullptr;
		}
		template<typename TELEMENT, typename TSEQUENCE>
		TELEMENT* FindElement( TSEQUENCE& Instance, size_t Index ) const {
			if( TypeResolver<TELEMENT>::Get() == ElementType ) return (TELEMENT const*)FindElement( &Instance, Index );
			else return nullptr;
		}
	};

	template<typename TELEMENT, typename TSEQUENCE>
	struct TSequenceTypeInfo : public SequenceTypeInfo
	{
		TSequenceTypeInfo() = delete;
		TSequenceTypeInfo( void (*InInitializer)( TypeInfo* ) )
		: SequenceTypeInfo( InInitializer )
		{}
		virtual ~TSequenceTypeInfo() {}

		virtual size_t GetCount( void const* Instance ) const override {
			return static_cast<TSEQUENCE const*>( Instance )->size();
		}

		virtual void const* FindElement( void const* Instance, size_t Index ) const override {
			return &static_cast<TSEQUENCE const*>( Instance )->operator[]( Index );
		}
	};
}
