#pragma once
#include "Engine/Archive.h"
#include "Engine/Flags.h"
#include "Engine/Logging.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Engine/StringID.h"
#include "ThirdParty/yaml.h"

namespace Resources {
	DECLARE_LOG_CATEGORY(Resources);
	
	/** Flag values which can apply to a particular resource */
	enum class EResourceFlags : uint32_t {
		/** Indicates this resource has been fully initialized */
		Initialized,
	};
	using FResourceFlags = TFlags<EResourceFlags>;

	struct Identifier {
		/** The name of the package in which the resource can be found */
		StringID package = StringID::None;
		/** The name of the resource that this identifier points to */
		StringID resource = StringID::None;

		inline bool operator==(Identifier const& other) const = default;
		inline bool operator!=(Identifier const& other) const = default;
		inline bool operator<(Identifier const& other) const {
			if (package == other.package) return StringID::LexicalLess(resource, other.resource);
			else if (StringID::LexicalLess(package, other.package)) return true;
			else return false;
		}
	};
}

template<>
struct std::hash<Resources::Identifier> {
	inline size_t operator()(Resources::Identifier const& identifier) const {
		std::hash<StringID> const hasher;
		return stdext::hash_combine(hasher(identifier.package), hasher(identifier.resource));
	}
};

namespace Archive {
	template<>
	struct Serializer<Resources::Identifier> {
		static void Write(Output& archive, Resources::Identifier const& identifier);
		static void Read(Input& archive, Resources::Identifier& identifier);
	private:
		static void WriteDirect(Output& archive, Resources::Identifier const& identifier);
	};
}

namespace YAML {
	template<>
	struct convert<Resources::Identifier> {
		static Node encode(Resources::Identifier const& identifier);
		static bool decode(Node const& node, Resources::Identifier& identifier);
	private:
		static Node EncodeDirect(Resources::Identifier const& identifier);
	};
}
