#include "Resources/Resource.h"
#include "Engine/Core.h"
#include "Resources/Package.h"
#include "Resources/Streaming.h"
using namespace Reflection;

DEFINE_STRUCT_REFLECTION_MEMBERS(Resources, Resource, "An object which can be shared between several other objects", {});

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

	IResourceProvider const* IResourceProvider::scoped_provider = nullptr;
}

namespace Archive {
	void ResourcePointerSerializer::WriteHandle(Output& archive, std::shared_ptr<Resources::Resource> const& handle) {
		Serializer<Resources::Identifier>::Write(archive, handle ? handle->GetIdentifier() : Resources::Identifier{});
	}
	std::shared_ptr<Resources::Resource> ResourcePointerSerializer::ReadHandle(Input& archive) {
		using namespace Resources;
		Identifier identifier;
		Serializer<Identifier>::Read(archive, identifier);

		if (identifier == Identifier{}) {
			return nullptr;
		}

		//Reading a resource this way does not load the resource, it can only find it if it's currently loaded.
		//The resource is a dependency, and dependencies should be loaded before getting to this point by separate means.
		if (auto const* provider = IResourceProvider::GetResourceProvider()) {
			if (std::shared_ptr<Resources::Resource> resource = provider->FindResource(identifier)) {
				return resource;
			} else {
				LOG(Resources, Warning, "Could not find resource for identifier %s. The handle will be empty.", identifier);
				return nullptr;
			}
		} else {
			LOG(Resources, Warning, "Attempted to deserialize a resource handle from an archive, but the current scope has no resource provider. The handle will be empty.");
			return nullptr;
		}
	}
}

namespace YAML {
	Node ResourcePointerConverter::EncodeHandle(std::shared_ptr<Resources::Resource> const& handle) {
		return convert<Resources::Identifier>::encode(handle ? handle->GetIdentifier() : Resources::Identifier{});
	}
	std::shared_ptr<Resources::Resource> ResourcePointerConverter::DecodeHandle(Node const& node) {
		using namespace Resources;
		const Identifier identifier = node.as<Identifier>();

		if (identifier == Identifier{}) {
			return nullptr;
		}

		//Reading a resource this way does not load the resource, it can only find it if it's currently loaded.
		//The resource is a dependency, and dependencies should be loaded before getting to this point by separate means.
		if (auto const* provider = IResourceProvider::GetResourceProvider()) {
			if (std::shared_ptr<Resources::Resource> resource = provider->FindResource(identifier)) {
				return resource;
			}
			else {
				LOG(Resources, Warning, "Could not find resource for identifier %s. The handle will be empty.", identifier);
				return nullptr;
			}
		}
		else {
			LOG(Resources, Warning, "Attempted to deserialize a resource handle from an archive, but the current scope has no resource provider. The handle will be empty.");
			return nullptr;
		}
	}
}
