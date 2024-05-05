#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Database.h"
#include "Resources/Package.h"
#include "Resources/Resource.h"

namespace Resources {
	/** Packages exist only in memory, and are not loaded from any persistent source. They can be modified arbitrarily. */
	struct MemoryDatabase : public Database {
		/** Create a new empty package with the provided name. If a package with this name already exists, that will be returned instead. */
		std::shared_ptr<Package> CreatePackage(StringID name) { return Database::CreatePackage(name); }
		/** Destroy an existing package by name. Will throw if the package cannot be destroyed because it is being used */
		void DestroyPackage(StringID name) { return Database::DestroyPackage(name); }

		/** Find an existing package by name */
		std::shared_ptr<Package const> FindPackage(StringID name) const { return Database::FindPackage(name); }
		std::shared_ptr<Package> FindPackage(StringID name) { return Database::FindPackage(name); }

		/** Create a new resource in the provided package. Will throw if the resource cannot be created. */
		template<Concepts::DerivedFromResource T, std::invocable<T&> InitializerType = stdext::no_op<T&>>
		stdext::shared_ref<T> Create(StringID name, stdext::shared_ref<Package> package, InitializerType&& initializer = stdext::no_op<T&>{}) { return Database::Create<T>(name, package, std::forward<InitializerType>(initializer)); }
	};
}
