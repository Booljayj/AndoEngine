#include "Resources/Package.h"
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"
#include "Resources/PackageDatabase.h"
#include "Resources/Resource.h"

namespace Resources {
	void Package::Rename(StringID newName) {
		auto const pinned = owner.lock();
		if (!pinned) {
			throw FormatType<std::runtime_error>("Cannot rename {} to {}, this package does not have an owning database. It is either a global package, or the database was already destroyed", name, newName);
		}
		
		pinned->RenamePackage(*this, newName);
	}

	bool Package::Contains(StringID resourceName) const {
		auto const contents = thread.contents.LockInclusive();
		return contents->contains(resourceName);
	}

	Handle<Resource> Package::Find(StringID resourceName) const {
		auto const contents = thread.contents.LockInclusive();
		if (auto const iter = contents->find(resourceName); iter != contents->end()) return iter->second.lock();
		else return nullptr;
	}

	void Package::Add(Reference<Resource> const& resource) {
		auto package = resource->thread.package.LockExclusive();
		auto contents = thread.contents.LockExclusive();

		//Ensure the resource doesn't already belong to another package. If it already belongs to this package, we can return early
		if (*package) {
			if (package->get() == this) return;
			else throw FormatType<std::runtime_error>("Cannot add {} to package {}, it is already part of package {}. It must be removed before it can be added to another package.", resource->name, name, (*package)->name);
		}

		//Ensure the package doesn't already contain a resource with the same name. Names must be unique within the same package.
		if (auto const iter = contents->find(resource->name); iter != contents->end()) {
			throw FormatType<std::runtime_error>("Cannot add {} to package {}, this package already contains a resource with this name", resource->name, name);
		}

		//Add to this package and update the resource's package information
		contents->emplace(std::make_pair(resource->name, resource.get()));
		(*package) = shared_from_this();

		{
			FPackageFlags expected = flags.load();
			while (flags.compare_exchange_weak(expected, expected + EPackageFlags::Dirty)) {};
		}
	}

	void Package::Remove(Reference<Resource> const& resource) {
		auto package = resource->thread.package.LockExclusive();
		auto contents = thread.contents.LockExclusive();

		auto const iter = contents->find(resource->name);
		if (iter == contents->end()) {
			throw FormatType<std::runtime_error>("Cannot remove {} from package {}, this package doesn't contain a resource with this name", resource->name, name);
		}

		//Create a local copy to make sure the resource isn't destroyed before we're finished modifying it.
		auto const pinned = iter->second.lock();
		if (pinned != resource.get()) {
			throw FormatType<std::runtime_error>("Cannot remove {} from package {}, this package doesn't contain this resource", resource->name, name);
		}

		//Remove from this package and update the resource's package information
		contents->erase(iter);
		package->reset();

		{
			FPackageFlags expected = flags.load();
			while (flags.compare_exchange_weak(expected, expected + EPackageFlags::Dirty)) {}
		}
	}

	void Package::Mount(Reference<Resource> const& resource) {
		auto contents = thread.contents.LockExclusive();

		auto const iter = contents->find(resource->name);
		if (iter == contents->end()) {
			throw FormatType<std::runtime_error>("Cannot mount {} on package {}, this package doesn't contain a resource with this name", resource->name, name);
		}

		auto const pinned = iter->second.lock();
		if (pinned != resource.get()) {
			throw FormatType<std::runtime_error>("Cannot mount {} on package {}, this resource is already mounted with a different instance", resource->name, name);
		}

		iter->second = resource.get();
	}

	void Package::Unmount(StringID resourceName) {
		auto contents = thread.contents.LockExclusive();

		auto const iter = contents->find(resourceName);
		if (iter == contents->end()) {
			throw FormatType<std::runtime_error>("Cannot unmount {} on package {}, this package doesn't contain a resource with this name", resourceName, name);
		}

		std::weak_ptr<Resource> empty;
		if (!iter->second.owner_before(empty) && !empty.owner_before(iter->second)) {
			throw FormatType<std::runtime_error>("Cannot unmount {} on package {}, this resource is not mounted", resourceName, name);
		}

		iter->second.reset();
	}

	void Package::RenameResource(Resource& resource, StringID newName) {
		auto contents = thread.contents.LockExclusive();

		if (resource.name == newName) return; //The name of a resource is access-restricted by the mutex on the package
		if (contents->contains(newName)) {
			throw FormatType<std::runtime_error>("Cannot rename {} to {}, the containing package {} alreaady contains a resource with this name.", resource.name, newName, name);
		}

		contents->emplace(newName, resource.shared_from_this());
		contents->erase(resource.name);
		resource.name = newName;
	}
}
