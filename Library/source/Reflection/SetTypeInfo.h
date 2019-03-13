#pragma once
#include <vector>
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct SetTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Set;

		/** The type of the values in the set */
		TypeInfo const* ValueType = nullptr;

		SetTypeInfo() = delete;
		SetTypeInfo(
			sid_t InUniqueID, CompilerDefinition InDefinition,
			char const* InDescription, Serialization::ISerializer* InSerializer,
			TypeInfo const* InValueType
		);
		virtual ~SetTypeInfo() {}

		/** Get the number of values that are in the set*/
		virtual size_t GetCount( void const* Instance ) const = 0;

		/** Get a vector of all the values in the set */
		virtual void GetValues( void const* Instance, std::vector<void const*>& OutValues ) const = 0;

		/** Returns true if an element with an equal value is contained in the set */
		virtual bool Contains( void const* Instance, void const* Value ) const = 0;

		/** Remove all values from the set */
		virtual void Clear( void* Instance ) const = 0;
		/** Adds the value to the set */
		virtual bool Add( void* Instance, void const* Value ) const = 0;
		/** Remove the value from the set */
		virtual bool Remove( void* Instance, void const* Value ) const = 0;
	};

	//============================================================
	// Templates

	template<typename TVALUE, typename TSET>
	struct TSetTypeInfo : public SetTypeInfo {
		TSetTypeInfo( char const* InDescription, Serialization::ISerializer* InSerializer )
		: SetTypeInfo(
			TypeResolver<TSET>::GetID(), GetCompilerDefinition<TSET>(),
			InDescription, InSerializer,
			TypeResolver<TVALUE>::Get() )
		{}

		static constexpr TSET const& CastSet( void const* Instance ) { return *static_cast<TSET const*>( Instance ); }
		static constexpr TSET& CastSet( void* Instance ) { return *static_cast<TSET*>( Instance ); }
		static constexpr TVALUE const& CastValue( void const* Value ) { return *static_cast<TVALUE const*>( Value ); }

		virtual void Construct( void* P ) const final { new (P) TSET; }
		virtual void Destruct( void* P ) const final { CastSet(P).~TSET(); }
		virtual bool Equal( void const* A, void const* B ) const final { return CastSet(A) == CastSet(B); }

		virtual size_t GetCount( void const* Instance ) const final { return CastSet( Instance ).size(); }

		virtual void GetValues( void const* Instance, std::vector<void const*>& OutValues ) const final {
			OutValues.clear();
			for( TVALUE const& Value : CastSet( Instance ) ) {
				OutValues.push_back( &Value );
			}
		}

		virtual bool Contains( void const* Instance, void const* Value ) const final {
			return CastSet( Instance ).find( CastValue( Value ) ) != CastSet( Instance ).cend();
		}

		virtual void Clear( void* Instance ) const final { CastSet( Instance ).clear(); }
		virtual bool Add( void* Instance, void const* Value ) const final { return CastSet( Instance ).insert( CastValue( Value ) ).second; }
		virtual bool Remove( void* Instance, void const* Value ) const final { return CastSet( Instance ).erase( CastValue( Value ) ) > 0; }
	};
}
