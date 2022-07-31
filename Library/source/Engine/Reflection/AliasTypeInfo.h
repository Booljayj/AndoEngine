#pragma once
#include "Engine/Reflection/Components/VariableInfo.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	/** An alias is a struct with a single member that behaves as if it was that member */
	struct AliasTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Alias;

		virtual ~AliasTypeInfo() = default;

		/** The variable that contains the type being aliased */
		VariableInfo variable;
	};

	//============================================================
	// Templates

	template<typename Type>
	struct TAliasTypeInfo : public ImplementedTypeInfo<Type, AliasTypeInfo> {
		using AliasTypeInfo::variable;

		TAliasTypeInfo(std::string_view inName) : ImplementedTypeInfo<Type, AliasTypeInfo>(Reflect<Type>::ID, inName) {}

		TYPEINFO_BUILDER_METHODS(Type);
		inline decltype(auto) Variable(VariableInfo const& inVariable) { variable = inVariable; return *this; }
	};
}

TYPEINFO_REFLECT(Alias);
