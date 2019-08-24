#pragma once
#include "Reflection/Components/VariableInfo.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	/** An alias is a struct with a single member that behaves as if it was that member */
	struct AliasTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Alias;

		AliasTypeInfo() = delete;
		AliasTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			VariableInfo const* InAliasedVariableInfo
		);
		virtual ~AliasTypeInfo() = default;

		/** The variable that contains the type being aliased */
		VariableInfo const* AliasedVariableInfo = nullptr;
	};

	//============================================================
	// Templates

	template<typename AliasType>
	struct TAliasTypeInfo : public AliasTypeInfo {
		TAliasTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			VariableInfo const* InAliasedVariableInfo)
		: AliasTypeInfo(
			TypeResolver<AliasType>::GetID(), GetCompilerDefinition<AliasType>(),
			InDescription, InFlags, InSerializer,
			InAliasedVariableInfo)
		{}

		STANDARD_TYPEINFO_METHODS(AliasType)
	};
}
