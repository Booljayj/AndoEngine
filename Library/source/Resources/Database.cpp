#include "Resources/Database.h"
#include "Resources/Package.h"
#include "Resources/RegisteredResource.h"
#include "Resources/Resource.h"

namespace Resources {
	std::shared_ptr<Package> const Database::transient = std::make_shared<Package>(nullptr, StringID::None);

	bool Database::ContainsPackage(StringID name) const noexcept {
		auto const packages = ts_packages.LockInclusive();
		return packages->contains(name);
	}

	std::shared_ptr<Resource> Database::FindResource(Identifier id) const noexcept {
		auto const packages = ts_packages.LockInclusive();
		if (auto const package_iter = packages->find(id.package); package_iter != packages->end()) {
			auto const contents = package_iter->second->ts_contents.LockInclusive();
			if (auto const resource_iter = contents->find(id.resource); resource_iter != contents->end()) {
				return resource_iter->second;
			}
		}
		return nullptr;
	}

	std::shared_ptr<Package> Database::FindPackage(StringID name) const noexcept {
		if (name == StringID::None) return GetTransient();

		auto const packages = ts_packages.LockInclusive();
		if (auto const iter = packages->find(name); iter != packages->end()) return iter->second;
		else return nullptr;
	}

	std::shared_ptr<Package> Database::CreatePackage(StringID name) {
		if (name == StringID::None) throw FormatType<std::runtime_error>("Cannot create package named {}, this is a reserved name.", name);

		auto packages = ts_packages.LockExclusive();
		if (auto const iter = packages->find(name); iter != packages->end()) {
			throw FormatType<std::runtime_error>("Cannot create package named {}, there is already a package with this name.", name);
		} else {
			if (!CanCreatePackage(name)) throw FormatType<std::runtime_error>("Cannot create package nameed {}. This package likely already exists, but is not loaded.", name);

			auto const result = packages->emplace(make_pair(name, make_shared<Package>(shared_from_this(), name)));
			return result.first->second;
		}
	}

	std::shared_ptr<Package> Database::FindOrCreatePackage(StringID name) {
		if (name == StringID::None) return GetTransient();

		auto packages = ts_packages.LockExclusive();
		if (auto const iter = packages->find(name); iter != packages->end()) return iter->second;
		else {
			if (!CanCreatePackage(name)) throw FormatType<std::runtime_error>("Cannot create package nameed {}. This package likely already exists, but is not loaded.", name);

			auto const result = packages->emplace(make_pair(name, make_shared<Package>(shared_from_this(), name)));
			return result.first->second;
		}
	}

	std::shared_ptr<Package> Database::CreatePackageWithContents(StringID name, std::unordered_map<StringID, std::shared_ptr<Resource>> const& contents) {
		if (name == StringID::None) throw FormatType<std::runtime_error>("Cannot create package named {}, this is a reserved name.", name);

		auto packages = ts_packages.LockExclusive();
		if (auto const iter = packages->find(name); iter != packages->end()) {
			throw FormatType<std::runtime_error>("Cannot create package named {}, there is already a package with this name.", name);
		} else {
			if (!CanCreatePackage(name)) throw FormatType<std::runtime_error>("Cannot create package named {}. This package likely already exists, but is not loaded.", name);

			auto const result = packages->emplace(make_pair(name, make_shared<Package>(shared_from_this(), name, contents)));
			return result.first->second;
		}
	}

	void Database::DestroyPackage(StringID name) {
		//The transient package cannot be destroyed
		if (name == StringID::None) throw FormatType<std::runtime_error>("Unable to destroy package {}, it is a global package that is never destroyed", name);

		auto packages = ts_packages.LockExclusive();

		auto const iter = packages->find(name);
		if (iter == packages->end()) return;
		if (iter->second.use_count() > 1) throw FormatType<std::runtime_error>("Unable to destroy package {}, it is being referenced by an external handle", name);

		{
			//Create a local copy within this scope, and lock the contents to make sure they are not changed before we erase the package.
			//We must make a local copy or the package will be destroyed before we can unlock it.
			std::shared_ptr<Package> const package = iter->second;
			auto const contents = package->ts_contents.LockExclusive();

			if (contents->size() > 0) throw FormatType<std::runtime_error>("Unable to destroy package {}, it is not empty", name);
			packages->erase(iter);
		}
	}

	std::shared_ptr<Cache> Database::FindCache(Reflection::StructTypeInfo const& type) {
		auto const caches = ts_caches.LockInclusive();

		auto iter = caches->find(type.id);
		if (iter != caches->end()) return iter->second;
		return nullptr;
	}

	stdext::shared_ref<Cache> Database::FindOrCreateCache(Reflection::StructTypeInfo const& type) {
		auto caches = ts_caches.LockExclusive();

		auto iter = caches->find(type.id);
		if (iter != caches->end()) return iter->second;

		auto const* utility = RegisteredResource::FindUtilities(type.id);
		if (!utility) throw FormatType<std::runtime_error>("Unable to create unknown resource {}, this is not a registered resource type.", type.name);

		stdext::shared_ref<Cache> cache = utility->CreateCache();
		caches->emplace(std::make_pair(type.id, cache));
		return cache;
	}
}
