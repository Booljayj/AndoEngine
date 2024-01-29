#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/StringID.h"
#include "Engine/Threads.h"

namespace Resources {
	struct Package;
	
	/** Base class for package databases, which manage a set of packages. Protected methods can be exposed by derived type as needed. */
	struct PackageDatabase : std::enable_shared_from_this<PackageDatabase> {
	public:
		/** Returns true if the database contains an existing package with the provided name */
		bool Contains(StringID name) const;

	protected:
		/** Find an existing package by name */
		std::shared_ptr<Package> Find(StringID name) const;
		/** Create a new empty package with the provided name. If a package with this name already exists, that will be returned instead. */
		std::shared_ptr<Package> Create(StringID name);

		/** Destroy an existing package by name. Will throw if the package cannot be destroyed because it is being used */
		void Destroy(StringID name);

	private:
		friend Package;

		struct {
			ThreadSafe<std::unordered_map<StringID, stdext::not_null<std::shared_ptr<Package>>>> packages;
		} thread;

		void RenamePackage(Package& package, StringID newName);
	};
}
