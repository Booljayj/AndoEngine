#pragma once
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	//@note "Poly" is short for Polymorphic Interface. A poly holds an optional instance that can be cast to a base type.

	struct PolyTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Poly;

		/** The base type for this poly */
		TypeInfo const* BaseType = nullptr;
		/** Whether this poly can hold a value of the base type */
		uint8_t CanBeBaseType : 1;
		/** Whether this poly can hold a value that derives from the base type */
		uint8_t CanBeDerivedType : 1;

		PolyTypeInfo() = delete;
		PolyTypeInfo(
			sid_t InUniqueID, CompilerDefinition InDefinition,
			char const* InDescription, Serialization::ISerializer* InSerializer,
			TypeInfo const* InBaseType, bool InCanBeBaseType, bool InCanBeDerivedType
		);
		virtual ~PolyTypeInfo() {}

		static bool CanAssignType( PolyTypeInfo const* PolyInfo, TypeInfo const* Info );

		/** Get the current value of the poly. Can be nullptr if the poly is unassigned */
		virtual void* GetValue( void* Instance ) const = 0;
		virtual void const* GetValue( void const* Instance ) const = 0;

		/** Assign a new value to a poly */
		virtual bool Assign( void* Instance, TypeInfo const* Type, void const* Value ) const = 0;
		/** Unassign the value of a poly */
		virtual void Unassign( void* Instance ) const = 0;
	};

	//============================================================
	// Templates

	template<typename TBASE>
	struct TUniquePtrTypeInfo : public PolyTypeInfo {
		static_assert( !std::is_void<TBASE>::value, "std::unique_ptr<void> is not supported for reflection. Reflecting the internal value is inherently impossible, which makes it unsafe to use." );
		using TPTR = std::unique_ptr<TBASE>;

		TUniquePtrTypeInfo( char const* InDescription, Serialization::ISerializer* InSerializer )
		: PolyTypeInfo(
			TypeResolver<TPTR>::GetID(), GetCompilerDefinition<TPTR>(),
			InDescription, InSerializer,
			TypeResolver<TBASE>::Get(), !std::is_abstract<TBASE>::value, std::is_class<TBASE>::value && !std::is_final<TBASE>::value )
		{}

		static constexpr TPTR const& CastPtr( void const* Instance ) { return *static_cast<TPTR const*>( Instance ); }
		static constexpr TPTR& CastPtr( void* Instance ) { return *static_cast<TPTR*>( Instance ); }
		static constexpr TBASE const& CastBase( void const* Base ) { return *static_cast<TBASE const*>( Base ); }

		virtual void Construct( void* P ) const final { new (P) TPTR; }
		virtual void Destruct( void* P ) const final { static_cast<TPTR*>(P)->~TPTR(); }
		virtual bool Equal( void const* A, void const* B ) const final { return CastPtr(A) == CastPtr(B); }

		virtual void* GetValue( void* Instance ) const final { return CastPtr( Instance ).get(); }
		virtual void const* GetValue( void const* Instance ) const final { return CastPtr( Instance ).get(); }

		virtual bool Assign( void* Instance, TypeInfo const* Type, void const* Value ) const final {
			if( CanAssignType( this, Type ) ) {
				//@todo Value is not used yet. We'll need a way to add standard copy-constructors to the base TypeInfo class, or a two-param Construct method.
				void* NewInstance = std::malloc( Type->Definition.Size );
				Type->Construct( NewInstance );
				CastPtr( Instance ).reset( reinterpret_cast<TBASE*>( NewInstance ) );
				return true;
			} else {
				return false;
			}
		}
		virtual void Unassign( void* Instance ) const final {
			CastPtr( Instance ).reset();
		}
	};
}
