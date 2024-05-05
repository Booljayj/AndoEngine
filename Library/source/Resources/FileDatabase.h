#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Database.h"

namespace Resources {
	/** Packages correspond to files on disk. They can be modified arbitrarily. */
	struct FileDatabase : public Database {
		/** Create a new empty package with the provided name. If a package with this name already exists, that will be returned instead. Throws if this package exists but is not loaded. */
		std::shared_ptr<Package> CreatePackage(StringID name) { return Database::CreatePackage(name); }
		/** Load an existing package from disk. Will throw if the package does not exist. If the package is already loaded, that will be returned instead. */
		std::shared_ptr<StreamingPackage> LoadPackage(StringID name) { return Database::LoadPackage(name); }
		/** Destroy an existing package by name. Will throw if the package cannot be destroyed because it is being used. */
		void DestroyPackage(StringID name) { return Database::DestroyPackage(name); }

		/** Find an existing package by name */
		std::shared_ptr<Package const> FindPackage(StringID name) const { return Database::FindPackage(name); }
		std::shared_ptr<Package> FindPackage(StringID name) { return Database::FindPackage(name); }

		/** Returns true if the package has saved data on the filesystem. Does not check if the package is loaded or dirty. */
		bool IsPackageSaved(StringID name) const;

		/** Save changes to a package to disk. Will create a file for the package if it doesn't already exist. */
		void SavePackage(stdext::shared_ref<Package> package);
		/** Reload the package from disk, discarding any unsaved changes that were made to the package. Does nothing if the package has no on-disk representation. */
		void ReloadPackage(stdext::shared_ref<Package> package);
		/**
		 * Delete the on-disk representation for a package.
		 * If the package is loaded, this will not affect the actual package, but after calling this the package cannot be loaded again until it is recreated.
		 * Does nothing if the package doesn't exist on-disk.
		 */
		void DeletePackage(StringID name);

		/** Create a new resource in the provided package. Will throw if the resource cannot be created. */
		template<Concepts::DerivedFromResource T, std::invocable<T&> InitializerType = stdext::no_op<T&>>
		stdext::shared_ref<T> Create(StringID name, stdext::shared_ref<Package> package, InitializerType&& initializer = stdext::no_op<T&>{}) { return Database::Create<T>(name, package, std::forward<InitializerType>(initializer)); }
	
	protected:
		virtual PackageSource LoadPackageSource(StringID name) final;

		virtual bool CanCreatePackage(StringID name) final;
		
	private:
		/** Convert a package name to a filesystem path where the package can be found */
		static std::filesystem::path GetPath(StringID name);
	};
}
