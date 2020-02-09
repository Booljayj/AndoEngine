#pragma once
#include "Reflection/Components/VariableInfo.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	/** An alias is a struct with a single member that behaves as if it was that member */
	struct AliasTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classificiation = ETypeClassification::Alias;

		AliasTypeInfo() = delete;
		AliasTypeInfo(
			Hash128 inID, CompilerDefinition inDef,
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			VariableInfo const* inVariableInfo
		);
		virtual ~AliasTypeInfo() = default;

		/** The variable that contains the type being aliased */
		VariableInfo const* variableInfo = nullptr;
	};

	//============================================================
	// Templates

	template<typename AliasType>
	struct TAliasTypeInfo : public AliasTypeInfo {
		TAliasTypeInfo(
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			VariableInfo const* inVariableInfo)
		: AliasTypeInfo(
			TypeResolver<AliasType>::GetID(), GetCompilerDefinition<AliasType>(),
			inDescription, inFlags, inSerializer,
			inVariableInfo)
		{}

		STANDARD_TYPEINFO_METHODS(AliasType)
	};
}
