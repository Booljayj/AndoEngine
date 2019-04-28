#pragma once
#include <tuple>
#include "Engine/TupleUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TupleTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Tuple;

		/** The number of elements in the tuple */
		size_t Size = 0;

		TupleTypeInfo() = delete;
		TupleTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			char const* InDescription, Serialization::ISerializer* InSerializer,
			size_t InSize
		);
		virtual ~TupleTypeInfo() {}

		/** Get the type of the value at a specific index in the tuple */
		virtual TypeInfo const* GetValueType( size_t Index ) const = 0;

		/** Get the value at a specific index in the tuple */
		virtual void* GetValue( void* Instance, size_t Index ) const = 0;
		virtual void const* GetValue( void const* Instance, size_t Index ) const = 0;
	};

	//============================================================
	// Templates

	/** Tuple element visitor which returns a type-erased pointer to the element */
	struct PointerVisitor {
		template<typename T>
		void* operator()( T& Element ) { return static_cast<void*>( &Element ); }
		template<typename T>
		void const* operator()( T const& Element ) { return static_cast<void const*>( &Element ); }
	};

	template<typename TTUPLE, typename ... TELEMENTS>
	struct TTupleTypeInfo : public TupleTypeInfo {
		TTupleTypeInfo( char const* InDescription, Serialization::ISerializer* InSerializer )
		: TupleTypeInfo(
			TypeResolver<TTUPLE>::GetID(), GetCompilerDefinition<TTUPLE>(),
			InDescription, InSerializer,
			std::tuple_size<TTUPLE>::value )
		{}

		static constexpr TTUPLE const& CastTuple( void const* Instance ) { return *static_cast<TTUPLE const*>( Instance ); }
		static constexpr TTUPLE& CastTuple( void* Instance ) { return *static_cast<TTUPLE*>( Instance ); }

		virtual void Construct( void* P ) const final { new (P) TTUPLE; }
		virtual void Destruct( void* P ) const final { CastTuple(P).~TTUPLE(); }
		virtual bool Equal( void const* A, void const* B ) const final { return CastTuple(A) == CastTuple(B); }

		virtual TypeInfo const* GetValueType( size_t Index ) const final {
			return TupleUtility::VisitTypeAt<TypeInfo const*, TTUPLE, TypeResolver>( Index );
		}

		virtual void* GetValue( void* Instance, size_t Index ) const final {
			return TupleUtility::VisitAt<void*>( CastTuple( Instance ), Index, PointerVisitor{} );
		}
		virtual void const* GetValue( void const* Instance, size_t Index ) const final {
			return TupleUtility::VisitAt<void const*>( CastTuple( Instance ), Index, PointerVisitor{} );
		}
	};
}
