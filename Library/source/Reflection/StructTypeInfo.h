#pragma once
#include <memory>
#include <vector>
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	struct StructTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Struct;

		StructTypeInfo() = delete;
		StructTypeInfo( void (*InInitializer)( TypeInfo* ), std::string&& InName, size_t InSize );
		virtual ~StructTypeInfo() {}

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo* BaseType = nullptr;
		/** Constants */
		std::vector<std::unique_ptr<StaticConstantInfo>> StaticConstants;
		std::vector<std::unique_ptr<MemberConstantInfo>> MemberConstants;
		/** Variables */
		std::vector<std::unique_ptr<StaticVariableInfo>> StaticVariables;
		std::vector<std::unique_ptr<MemberVariableInfo>> MemberVariables;
		/** Functions */
		//std::vector<std::unique_ptr<FunctionInfo>> Functions; //@todo This will change when the FunctionInfo implementation is better fleshed out.
		/** Actions */
		//functions without return values, essentially

		virtual void OnLoaded() override;

		/** Returns true if the chain of base types includes the provided type */
		bool DerivesFrom( TypeInfo const* Base ) const { return true; }

		/** Get a list of all constants, including those from base classes */
		void GetStaticConstantsRecursive( std::vector<StaticConstantInfo const*>& OutStaticConstants ) const {}
		void GetMemberConstantsRecursive( std::vector<MemberConstantInfo const*>& OutMemberConstants ) const {}
		/** Get a list of all variables, including those from base classes */
		void GetStaticVariablesRecursive( std::vector<StaticVariableInfo const*>& OutStaticVariables ) const {}
		void GetMemberVariablesRecursive( std::vector<MemberVariableInfo const*>& OutMemberVariables ) const {}

		//Find a constant that has the provided name hash
		StaticConstantInfo const* FindStaticConstantInfo( uint16_t NameHash ) const { return nullptr; }
		MemberConstantInfo const* FindMemberConstantInfo( uint16_t NameHash ) const { return nullptr; }
		//Find a variable that has the provided name hash
		StaticVariableInfo const* FindStaticVariableInfo( uint16_t NameHash ) const { return nullptr; }
		MemberVariableInfo const* FindMemberVariableInfo( uint16_t NameHash ) const { return nullptr; }
	};
}
