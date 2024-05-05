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

	namespace Concepts {
		template<typename T>
		concept ReflectedAlias = ReflectedType<T> and requires (T a) {
			{ ::Reflection::Reflect<T>::Get() } -> std::convertible_to<AliasTypeInfo const&>;
		};
	}

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

namespace YAML {
	template<Reflection::Concepts::ReflectedAlias Type>
	struct convert<Type> {
		static Node encode(const Type& instance) {
			Reflection::AliasTypeInfo const& type = Reflect<Type>::Get();
			Reflection::VariableInfo const& variable = type.variable;

			return variable.type->Serialize(variable.GetImmutable(&instance));
		}

		static bool decode(const Node& node, Type& instance) {
			Reflection::AliasTypeInfo const& type = Reflect<Type>::Get();
			Reflection::VariableInfo const& variable = type.variable;

			variable.type->Deserialize(node, variable.GetMutable(&instance));
			return true;
		}
	};
}
