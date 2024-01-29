#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Package.h"
#include "Resources/PackageDatabase.h"
#include "Resources/Resource.h"

namespace Resources {
	/** Packages exist only in memory, and are not loaded from any persistent source. They can be modified arbitrarily. */
	struct MemoryPackageDatabase : public PackageDatabase {
		/** Create a new empty package with the provided name. If a package with this name already exists, that will be returned instead. */
		std::shared_ptr<Package> Create(StringID name) { return PackageDatabase::Create(name); }
		/** Destroy an existing package by name. Will throw if the package cannot be destroyed because it is being used */
		void Destroy(StringID name) { return PackageDatabase::Destroy(name); }

		/** Find an existing package by name */
		std::shared_ptr<Package const> Find(StringID name) const { return PackageDatabase::Find(name); }
		std::shared_ptr<Package> Find(StringID name) { return PackageDatabase::Find(name); }

		/** Save changes to a package. Memory packages don't have an on-disk representation, but this will remove the Dirty flag as through changes were saved */
		void Save(stdext::not_null<std::shared_ptr<Package>> package);
	};
}
