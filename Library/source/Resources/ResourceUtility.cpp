#include "Resources/ResourceUtility.h"
#include "Resources/Database.h"
#include "Resources/Package.h"
#include "Resources/Resource.h"

namespace Resources {
	void ResourceUtility::MoveResource(stdext::shared_ref<Resource> resource, std::shared_ptr<Package> const& destination) {
		auto description = resource->ts_description.LockExclusive();

		std::shared_ptr<Package> const& current = description->package.lock();
		if (current != destination)
		{
			//Throws if the resource cannot be removed from the contents
			const auto VerifyResourceCanBeRemoved = [](std::shared_ptr<Resource> const& resource, Resource::ResourceDescription const& description, StringID name, Package::ContentsContainerType const& contents) {
				auto const iter = contents.find(description.name);

				if (iter == contents.end() || iter->second != resource) {
					throw FormatType<std::runtime_error>("Cannot remove {} from package {}, this package doesn't contain this resource", description.name, name);
				}
			};

			//Throws if the resource cannot be added to the contents
			const auto VerifyResourceCanBeAdded = [](Resource::ResourceDescription const& description, StringID name, Package::ContentsContainerType const& contents) {
				auto const iter = contents.find(description.name);

				if (iter != contents.end()) {
					throw FormatType<std::runtime_error>("Cannot add {} to package {}, this package already contains a resource with this name", description.name, name);
				}
			};

			//If this resource is currently in a package, then we need to remove from the old package and add to the new one
			if (current) {
				auto currentContents = current->ts_contents.LockExclusive();
				auto destinationContents = destination->ts_contents.LockExclusive();

				VerifyResourceCanBeRemoved(resource, *description, current->name, *currentContents);
				VerifyResourceCanBeAdded(*description, destination->name, *destinationContents);

				destinationContents->emplace(std::make_pair(description->name, resource.get()));
				currentContents->erase(description->name);
				description->package = destination;

				current->flags += EPackageFlags::Dirty;
				destination->flags += EPackageFlags::Dirty;

			//If this resource is not currently in a package, then we just need to add it to the new package
			} else {
				auto destinationContents = destination->ts_contents.LockExclusive();

				VerifyResourceCanBeAdded(*description, destination->name, *destinationContents);

				destinationContents->emplace(std::make_pair(description->name, resource.get()));
				description->package = destination;

				destination->flags += EPackageFlags::Dirty;
			}
		}
	}

	void ResourceUtility::RenamePackage(stdext::shared_ref<Package> package, StringID name) {
		auto const database = package->owner.lock();

		//Ensure tha the package actually belongs to a database that will allow us to ensure it's unique.
		if (!database) throw FormatType<std::runtime_error>("Cannot rename package {} to {}. This package is not part of a database. This means it's a global or orphaned package, which cannot be renamed.", package->name, name);

		auto packages = database->ts_packages.LockExclusive();
		
		//Ensure there isn't already a package with this name
		if (auto const iter = packages->find(name); iter != packages->end()) {
			throw FormatType<std::runtime_error>("Cannot rename package {} to {}. There is already a package with the new name.", package->name, name);
		}

		//Perform the rename
		packages->emplace(name, package);
		packages->erase(package->name);
		package->name = name;
	}

	void ResourceUtility::RenameResource(stdext::shared_ref<Resource> resource, StringID name) {
		auto description = resource->ts_description.LockExclusive();

		if (auto const package = description->package.lock()) {
			//This resource is within a package, so we must verify that the new name will still be unique within the package.
			auto contents = package->ts_contents.LockExclusive();

			//The name of a resource is access-restricted by the mutex on the package contents, so we still need to lock the
			//contents before we can even check if the name is the same and we should just return. Otherwise, if there's an
			//in-progress rename from a different thread, this could be a data race condition.
			if (description->name == name) return;

			if (contents->contains(name)) {
				throw FormatType<std::runtime_error>("Cannot rename {} to {}, the containing package {} already contains a resource with this name.", description->name, name, package->name);
			}

			//Update the package contents with the new name
			contents->emplace(std::make_pair(name, resource.get()));
			contents->erase(description->name);
			description->name = name;

		} else {
			//If the resource does not belong to a package, renaming it is trivial.
			description->name = name;
		}
	}
}
