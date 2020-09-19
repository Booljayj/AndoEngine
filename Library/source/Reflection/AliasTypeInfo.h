#pragma once
#include "Reflection/Components/VariableInfo.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	/** An alias is a struct with a single member that behaves as if it was that member */
	struct AliasTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classificiation = ETypeClassification::Alias;

		AliasTypeInfo() = delete;
		AliasTypeInfo(Hash128 inID, CompilerDefinition inDef);
		virtual ~AliasTypeInfo() = default;

		/** The variable that contains the type being aliased */
		VariableInfo const* variableInfo = nullptr;
	};

	//============================================================
	// Templates

	template<typename AliasType>
	struct TAliasTypeInfo : public AliasTypeInfo {
		TAliasTypeInfo()
		: AliasTypeInfo(TypeResolver<AliasType>::GetID(), GetCompilerDefinition<AliasType>())
		{}

		STANDARD_TYPEINFO_METHODS(AliasType)

		TAliasTypeInfo& VariableInfo(VariableInfo const* inVariableInfo) { variableInfo = inVariableInfo; return *this; }
	};
}
