#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/Hash.h"
#include "Engine/String.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	/** A reference to a registered TypeInfo object, used to help look up the corresponding instance */
	struct TypeInfoReference {
		/** Instance that registers a type so that it can be found by resolving a TypeInfoReference instance. */
		struct Registered {
			Registered(TypeInfo const& info);
			~Registered();
		private:
			TypeInfo const* cached;
		};

		std::u16string name;
		Hash128 id;

		TypeInfoReference() = default;
		TypeInfoReference(TypeInfo const& type) : name(type.name), id(type.id) {}

		/** Find the TypeInfo object that matches this reference */
		TypeInfo const* Resolve() const;

		/** Find the TypeInfo object that matches this reference with the expected type */
		template<std::derived_from<TypeInfo> Type>
		Type const* Resolve() const { return Cast<Type>(Resolve()); }

	private:
		static std::deque<TypeInfo const*> infos;
	};
}

namespace Archive {
	template<>
	struct Serializer<Reflection::TypeInfoReference> {
		static void Write(Output& archive, Reflection::TypeInfoReference const& value);
		static void Read(Input& archive, Reflection::TypeInfoReference& value);
	};
}

namespace YAML {
	template<>
	struct convert<Reflection::TypeInfoReference> {
		static Node encode(Reflection::TypeInfoReference const& value);
		static bool decode(Node const& node, Reflection::TypeInfoReference& value);
	};
}

template<>
struct std::formatter<Reflection::TypeInfoReference> : std::formatter<std::string_view> {
	inline auto format(const Reflection::TypeInfoReference& ref, format_context& ctx) const {
		return std::format_to(ctx.out(), "{}:{}", ref.name, ref.id);
	}
};
