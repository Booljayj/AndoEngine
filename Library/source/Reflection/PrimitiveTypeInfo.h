#include "Reflection/TypeInfo.h"
#include "Reflection/BaseResolver.h"

namespace Reflection {
	template<typename TYPE>
	struct TPrimitiveTypeInfo : public TypeInfo {
		TPrimitiveTypeInfo( char const* InDescription, Serialization::ISerializer* InSerializer )
		: TypeInfo(
			TypeInfo::CLASSIFICATION, TypeResolver<TYPE>::GetID(), GetCompilerDefinition<TYPE>(),
			InDescription, FTypeFlags::None, InSerializer )
		{}

		static constexpr TYPE const& Cast( void const* P ) { return *static_cast<TYPE const*>( P ); }
		static constexpr TYPE& Cast( void* P ) { return *static_cast<TYPE*>( P ); }

		virtual void Construct( void* P ) const final { new (P) TYPE; }
		virtual void Destruct( void* P ) const final { Cast(P).~TYPE(); }
		virtual bool Equal( void const* A, void const* B ) const final { return Cast(A) == Cast(B); }
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