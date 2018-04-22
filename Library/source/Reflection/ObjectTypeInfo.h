#pragma once
#include <memory>
#include <vector>
#include "Reflection/TypeInfo.h"
#include "Reflection/ConstantInfo.h"
#include "Reflection/VariableInfo.h"

namespace Reflection {
	struct ObjectTypeInfo : public TypeInfo
	{
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Primitive;

		ObjectTypeInfo() = delete;
		ObjectTypeInfo( void (*InInitializer)( TypeInfo* ) )
		: TypeInfo( InInitializer, CLASSIFICATION )
		{}
		virtual ~ObjectTypeInfo() {}

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		ObjectTypeInfo* BaseType = nullptr;
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
