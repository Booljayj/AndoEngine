#include "Reflection/Components/VariableInfo.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	/** An alias is a struct with a single member that behaves as if it was that member */
	struct AliasTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Alias;

		AliasTypeInfo() = delete;
		AliasTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			VariableInfo const* InVariable
		);
		virtual ~AliasTypeInfo() = default;

		/** The variable that contains the type being aliased */
		VariableInfo const* Variable = nullptr;
	};

	//============================================================
	// Templates

	template<typename AliasType>
	struct TAliasTypeInfo : public AliasTypeInfo {
		TAliasTypeInfo(char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer, VariableInfo const* InVariable)
		: AliasTypeInfo(
			TypeResolver<AliasType>::GetID(), GetCompilerDefinition<AliasType>(),
			InDescription, InFlags, InSerializer,
			InVariable)
		{}

		STANDARD_TYPEINFO_METHODS(AliasType)
	};
}
