#include "Reflection/TypeInfo.h"
#include "Reflection/BaseResolver.h"

namespace Reflection {
	template<typename TYPE>
	struct TPrimitiveTypeInfo : public TypeInfo {
		TPrimitiveTypeInfo( char const* InDescription, Serialization::ISerializer* InSerializer )
		: TypeInfo(
			TypeInfo::CLASSIFICATION, TypeResolver<TYPE>::GetID(), GetCompilerDefinition<TYPE>(),
			InDescription, FTypeFlags::HasCompare, InSerializer )
		{}

		static inline TYPE const* Cast( void const* P ) { return static_cast<TYPE const*>( P ); }
		static inline TYPE* Cast( void* P ) { return static_cast<TYPE*>( P ); }

		virtual void Construct( void* P ) const final { new (P) TYPE; }
		virtual void Destruct( void* P ) const final { Cast(P)->~TYPE(); }

		virtual bool Equal( void const* A, void const* B ) const final { return *Cast(A) == *Cast(B); }
		virtual int8_t Compare( void const* A, void const* B ) const final {
			if( *Cast(A) < *Cast(B) ) return -1;
			else if( *Cast(A) == *Cast(B) ) return 0;
			else return 1;
		};
	};

	/** Special type for void, which is not a type that can have values but still needs TypeInfo */
	template<> struct TPrimitiveTypeInfo<void> : public TypeInfo {
		TPrimitiveTypeInfo()
		: TypeInfo(
			TypeInfo::CLASSIFICATION, 0, GetCompilerDefinition<void>(),
			"not a type", FTypeFlags::None, nullptr )
		{}

		virtual void Construct( void* P ) const final {}
		virtual void Destruct( void* P ) const final {}
		virtual bool Equal( void const* A, void const* B ) const final { return false; }
	};
}