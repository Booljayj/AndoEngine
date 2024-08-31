#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Streaming.h"

namespace Resources {
	/** Packages correspond to files on disk. They can be modified arbitrarily. */
	struct FileDatabase : public StreamingDatabase {
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
		
		/** Save changes to a package to disk. Will create a file for the package if it doesn't already exist. */
		void SavePackage(StringID name) { StreamingDatabase::SavePackage(name); }

		/** Reload the package from disk, discarding any unsaved changes that were made to the package. Does nothing if the package has no on-disk representation. */
		void ReloadPackage(StringID name);

		/**
		 * Delete the on-disk representation for a package.
		 * If the package is loaded, this will not affect the actual package, but after calling this the package cannot be loaded again until it is recreated.
		 * Does nothing if the package doesn't exist on-disk.
		 */
		void DeletePackage(StringID name);

		/** Returns true if the package has saved data on the filesystem. Does not check if the package is loaded or dirty. */
		bool IsPackageSaved(StringID name) const;

	protected:
		virtual bool CanCreatePackage(StringID name) const override final;
		virtual bool SavePackage(Package const& package) override final;
		virtual PackageInput LoadPackageSource(StringID name) override final;

	private:
		/** Convert a package name to a filesystem path where the package can be found */
		static std::filesystem::path GetPath(StringID name);
	};
}
