#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Database.h"
#include "Resources/Package.h"
#include "Resources/Resource.h"

namespace Resources {
	/** Packages exist only in memory, and are not loaded from any persistent source. They can be modified arbitrarily. */
	struct MemoryDatabase : public Database {
		/** Create a new empty package with the provided name. If the package cannot be created, an exception will be thrown. */
		std::shared_ptr<Package> CreatePackage(StringID name) { return Database::CreatePackage(name); }
		/** Destroy an existing package by name. If the package is being used and cannot be destroyed, an exception will be thrown. */
		void DestroyPackage(StringID name) { return Database::DestroyPackage(name); }

		/** Find an existing package by name */
		std::shared_ptr<Package const> FindPackage(StringID name) const { return Database::FindPackage(name); }
		/** Find an existing package by name */
		std::shared_ptr<Package> FindPackage(StringID name) { return Database::FindPackage(name); }

		/** Create a new resource in the provided package. Will throw if the resource cannot be created. */
		template<Concepts::DerivedFromResource T, std::invocable<T&> InitializerType>
		stdext::shared_ref<T> Create(StringID name, stdext::shared_ref<Package> package, InitializerType&& initializer) { return Database::Create<T>(name, package, std::forward<InitializerType>(initializer)); }
	};
}
