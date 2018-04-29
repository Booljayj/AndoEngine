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
		StructTypeInfo( char const* InName, size_t InSize, void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InName, InSize, InInitializer, CLASSIFICATION )
		{}
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

		virtual void OnLoaded( bool bLoadDependencies ) override;

		bool DerivesFrom( TypeInfo const* Base ) const { return true; }

		//Find a constant that has the provided name hash
		StaticConstantInfo const* FindStaticConstantInfo( uint16_t NameHash ) { return nullptr; }
		MemberConstantInfo const* FindMemberConstantInfo( uint16_t NameHash ) { return nullptr; }
		//Find a variable that has the provided name hash
		StaticVariableInfo const* FindStaticVariableInfo( uint16_t NameHash ) { return nullptr; }
		MemberVariableInfo const* FindMemberVariableInfo( uint16_t NameHash ) { return nullptr; }
	};
}
