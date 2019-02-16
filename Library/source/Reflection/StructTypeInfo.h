#pragma once
#include <array>
#include <string_view>
#include <vector>
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

#define REFLECT( __TYPE__ )\
namespace Reflection {\
	template<> struct TypeResolver<__TYPE__> {\
		static TypeInfo const* Get() { return &__TYPE__::__TypeInfo__; }\
		static constexpr sid_t GetID() { return id( #__TYPE__ ); }\
	};\
}

#define REFLECTION_MEMBERS( __TYPE__ )\
static Reflection::TStructTypeInfo<__TYPE__> const __TypeInfo__;\
virtual Reflection::TypeInfo const* GetTypeInfo() const

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
			sid_t InUniqueID, size_t InSize, size_t InAlignment,
			const char* InMangledName, const char* InDescription,
			FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
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

	template<size_t NumConstants, size_t NumVariables>
	struct TFieldsStorage {
		std::array<ConstantInfo const*, NumConstants> Constants;
		std::array<VariableInfo const*, NumVariables> Variables;

		Fields MakeFields() const {
			Fields Result;
			Result.Constants = std::basic_string_view<ConstantInfo const*>{ Constants.data(), Constants.size() };
			Result.Variables = std::basic_string_view<VariableInfo const*>{ Variables.data(), Variables.size() };
			return Result;
		}
	};

	template<typename TYPE>
	struct TStructTypeInfo : public StructTypeInfo {
		TStructTypeInfo(
			const char* InDescription,
			FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			void const* InDefault, Fields InStatic, Fields InMember
		)
		: StructTypeInfo(
			TypeResolver<TYPE>::GetID(), sizeof( TYPE ), alignof( TYPE ),
			typeid( TYPE ).name(), InDescription,
			InFlags, InSerializer,
			InDefault, InStatic, InMember )
		{}

		static inline TYPE const* CastStruct( void const* P ) { return static_cast<TYPE const*>( P ); }
		static inline TYPE* CastStruct( void* P ) { return static_cast<TYPE*>( P ); }

		virtual void Construct( void* P ) const final { new (P) TYPE; }
		virtual void Destruct( void* P ) const final { CastStruct(P)->~TYPE(); }

		virtual bool Equal( void const* A, void const* B ) const final { return *CastStruct(A) == *CastStruct(B); }
		virtual int8_t Compare( void const* A, void const* B ) const final {
			if( *CastStruct(A) < *CastStruct(B) ) return -1;
			else if( *CastStruct(A) == *CastStruct(B) ) return 0;
			else return 1;
		};
	};
}
