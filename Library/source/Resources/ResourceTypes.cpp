#include "Resources/ResourceTypes.h"
#include "Resources/Streaming.h"
#include "Resources/StreamingUtils.h"

namespace Resources {
	DEFINE_LOG_CATEGORY(Resources, Info);
}

namespace Archive {
	void Serializer<Resources::Identifier>::Write(Output& archive, Resources::Identifier const& identifier) {
		if (Resources::CanSavePackage(identifier.package)) {
			WriteDirect(archive, identifier);
		} else {
			WriteDirect(archive, Resources::Identifier{});
		}
	}
	void Serializer<Resources::Identifier>::Read(Input& archive, Resources::Identifier& identifier) {
		Serializer<StringID>::Read(archive, identifier.package);
		Serializer<StringID>::Read(archive, identifier.resource);
	}

	void Serializer<Resources::Identifier>::WriteDirect(Output& archive, Resources::Identifier const& identifier) {
		Serializer<StringID>::Write(archive, identifier.package);
		Serializer<StringID>::Write(archive, identifier.resource);
	}
}

namespace YAML {
	Node convert<Resources::Identifier>::encode(Resources::Identifier const& identifier) {
		if (Resources::CanSavePackage(identifier.package)) {
			return EncodeDirect(identifier);
		} else {
			return Node{ NodeType::Sequence };
		}
	}
	bool convert<Resources::Identifier>::decode(Node const& node, Resources::Identifier& identifier) {
		if (node.IsSequence()) {
			if (node.size() == 0) {
				identifier = Resources::Identifier{};
				return true;
			}
			if (node.size() == 2) {
				identifier.package = node[0].as<StringID>();
				identifier.resource = node[1].as<StringID>();
				return true;
			}
		}
		return false;
	}

	Node convert<Resources::Identifier>::EncodeDirect(Resources::Identifier const& identifier) {
		Node node{ NodeType::Sequence };
		node.push_back(identifier.package);
		node.push_back(identifier.resource);
		return node;
	}
}