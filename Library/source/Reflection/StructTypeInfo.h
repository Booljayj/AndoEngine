#pragma once
#include <array>
#include <string_view>
#include <vector>
#include <type_traits>
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

/** Expose a struct to the reflection system. Must be expanded after the type is declared and includes the REFELCTION_MEMBERS macro */
#define REFLECT( __TYPE__ )\
namespace Reflection {\
	template<> struct TypeResolver<__TYPE__> {\
		static TypeInfo const* Get() { return &__TYPE__::_TypeInfo; }\
		static constexpr sid_t GetID() { return id( #__TYPE__ ); }\
	};\
}

/** Define members of the struct used for reflection. The second argument must be either the primary base class of this type or void */
#define REFLECTION_MEMBERS( __TYPE__, __BASE__ )\
using Super = __BASE__;\
static Reflection::TStructTypeInfo<__TYPE__> const _TypeInfo

namespace Reflection {
	/** Views into various field types that the struct defines */
	struct Fields {
		std::basic_string_view<ConstantInfo const*> Constants;
		std::basic_string_view<VariableInfo const*> Variables;
	};

	/** Info for a struct type, which contains various fields and supports inheritance */
	struct StructTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Struct;

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo const* BaseType = nullptr;
		/** A default-constructed instance of this struct type, used to find default values for variables */
		void const* Default = nullptr;
		/** The static fields that this type defines */
		Fields Static;
		/** The member fields that this type defines */
		Fields Member;

		StructTypeInfo() = delete;
		StructTypeInfo(
			sid_t InUniqueID, CompilerDefinition InDefinition,
			char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			StructTypeInfo const* InBaseType, void const* InDefault,
			Fields InStatic, Fields InMember
		);
		virtual ~StructTypeInfo() {}

		/** Returns true if the chain of base types includes the provided type */
		bool DerivesFrom( TypeInfo const* Base ) const { return true; }

		/** Get a list of all constants, including those from base classes */
		void GetStaticConstantsRecursive( std::vector<ConstantInfo const*>& OutStaticConstants ) const;
		void GetMemberConstantsRecursive( std::vector<ConstantInfo const*>& OutMemberConstants ) const;
		/** Get a list of all variables, including those from base classes */
		void GetStaticVariablesRecursive( std::vector<VariableInfo const*>& OutStaticVariables ) const;
		void GetMemberVariablesRecursive( std::vector<VariableInfo const*>& OutMemberVariables ) const;

		//Find a constant that has the provided name hash
		ConstantInfo const* FindStaticConstantInfo( uint16_t NameHash ) const { return nullptr; }
		ConstantInfo const* FindMemberConstantInfo( uint16_t NameHash ) const { return nullptr; }
		//Find a variable that has the provided name hash
		VariableInfo const* FindStaticVariableInfo( uint16_t NameHash ) const { return nullptr; }
		VariableInfo const* FindMemberVariableInfo( uint16_t NameHash ) const { return nullptr; }
	};

	//============================================================
	// Templates

	template<typename TYPE>
	struct TStructTypeInfo : public StructTypeInfo {
		using Super = typename TYPE::Super;
		static_assert( std::is_base_of<Super, TYPE>::value || std::is_void<Super>::value, "invalid type inheritance for StructTypeInfo, T::Super is not void or an actual base of T" );

		TStructTypeInfo(
			char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			void const* InDefault, Fields InStatic, Fields InMember
		)
		: StructTypeInfo(
			TypeResolver<TYPE>::GetID(), GetCompilerDefinition<TYPE>(),
			InDescription, InFlags, InSerializer,
			Reflection::Cast<StructTypeInfo>( TypeResolver<typename TYPE::Super>::Get() ), InDefault,
			InStatic, InMember )
		{}

		static constexpr TYPE const& Cast( void const* P ) { return *static_cast<TYPE const*>( P ); }
		static constexpr TYPE& Cast( void* P ) { return *static_cast<TYPE*>( P ); }

		virtual void Construct( void* P ) const final { new (P) TYPE; }
		virtual void Destruct( void* P ) const final { Cast(P).~TYPE(); }
		virtual bool Equal( void const* A, void const* B ) const final { return Cast(A) == Cast(B); }
	};
}
