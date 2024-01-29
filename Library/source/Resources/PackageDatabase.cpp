#include "Resources/PackageDatabase.h"
#include "Resources/Package.h"

namespace Resources {
	using namespace std;

	bool PackageDatabase::Contains(StringID name) const {
		auto const packages = thread.packages.LockInclusive();
		return packages->contains(name);
	}

	std::shared_ptr<Package> PackageDatabase::Find(StringID name) const {
		auto const packages = thread.packages.LockInclusive();
		if (auto const iter = packages->find(name); iter != packages->end()) return iter->second;
		else return nullptr;
	}

	std::shared_ptr<Package> PackageDatabase::Create(StringID name) {
		auto packages = thread.packages.LockExclusive();
		if (auto const iter = packages->find(name); iter != packages->end()) return iter->second;
		else {
			auto const result = packages->emplace(make_pair(name, make_shared<Package>(Package::PrivateToken{}, shared_from_this(), name)));
			return result.first->second;
		}
	}

	void PackageDatabase::Destroy(StringID name) {
		auto packages = thread.packages.LockExclusive();

		auto const iter = packages->find(name);
		if (iter == packages->end()) return;
		if (iter->second.get().use_count() > 1) throw FormatType<runtime_error>("Unable to destroy package {}, it is being referenced by an external handle", name);

		{
			//Create a local copy within this scope, and lock the contents to make sure they are not changed before we erase the package.
			//We must make a local copy or the package will be destroyed before we can unlock it.
			std::shared_ptr<Package> const package = iter->second;
			auto const contents = package->thread.contents.LockExclusive();

			if (contents->size() > 0) throw FormatType<runtime_error>("Unable to destroy package {}, it is not empty", name);
			packages->erase(iter);
		}
	}

	void PackageDatabase::RenamePackage(Package& package, StringID newName) {
		auto packages = thread.packages.LockExclusive();

		if (package.name == newName) return; //The name of a package is access-restricted by the mutex on the database
		if (packages->contains(newName)) {
			throw FormatType<std::runtime_error>("Cannot rename {} to {}, the database alreaady contains a package with this name.", package.name, newName);
		}

		packages->emplace(std::make_pair(newName, package.shared_from_this()));
		packages->erase(package.name);
		package.name = newName;
	}
}
