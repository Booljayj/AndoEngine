#include "Resources/Resource.h"
#include "Engine/StandardTypes.h"
#include "Resources/Package.h"
#include "Resources/Streaming.h"
using namespace Reflection;

DEFINE_REFLECT_STRUCT(Resources, Resource)
	.Description("An object which can be shared between several other objects");

namespace Resources {
	StringID Resource::GetName() const {
		auto const description = ts_description.LockInclusive();
		return description->name;
	}

	std::shared_ptr<Package const> Resource::GetPackage() const {
		auto const description = ts_description.LockInclusive();
		return description->package.lock();
	}

	Identifier Resource::GetIdentifier() const {
		auto const description = ts_description.LockInclusive();
		auto const pinned = description->package.lock();
		return Identifier{ pinned ? pinned->GetName() : StringID::None, description->name };
	}
}

namespace Archive {
	void Serializer<std::shared_ptr<Resources::Resource>>::Write(Output& archive, std::shared_ptr<Resources::Resource> const& handle) {
		Serializer<Resources::Identifier>::Write(archive, handle ? handle->GetIdentifier() : Resources::Identifier{});
	}
	void Serializer<std::shared_ptr<Resources::Resource>>::Read(Input& archive, std::shared_ptr<Resources::Resource>& handle) {
		Resources::Identifier identifier;
		Serializer<Resources::Identifier>::Read(archive, identifier);

		//@todo Use some kind of static scoped context to find the database and find the resource in the database.
		//      Unfortunately the design of the convert struct doesn't allow this to be naturally provided.
		handle = nullptr;
	}
}

namespace YAML {
	Node convert<std::shared_ptr<Resources::Resource>>::encode(std::shared_ptr<Resources::Resource> const& handle) {
		return convert<Resources::Identifier>::encode(handle ? handle->GetIdentifier() : Resources::Identifier{});
	}
	bool convert<std::shared_ptr<Resources::Resource>>::decode(Node const& node, std::shared_ptr<Resources::Resource>& handle) {
		const Resources::Identifier identifier = node.as<Resources::Identifier>();

		//@todo Use some kind of static scoped context to find the database and find the resource in the database.
		//      Unfortunately the design of the convert struct doesn't allow this to be naturally provided.
		handle = nullptr;
		return true;
	}
}
