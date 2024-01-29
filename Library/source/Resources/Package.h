#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/StringID.h"
#include "Resources/Resource.h"

namespace Resources {
	struct PackageDatabase;

	/** Flags that describe various states or metadata that apply to an individual package. Some flags are only used in specific contexts. */
	enum class EPackageFlags {
		/** This package has pending changes that should be saved */
		Dirty,
		/** This package was loaded from disk, rather than being newly created */
		Loaded,
	};
	using FPackageFlags = TFlags<EPackageFlags>;

	/**
	 * A package is an abstract representation of a collection of resources.
	 * It may correspond to a file, or to a location within an archive, or something else depending on the implementation.
	 * Each package must have a unique name, and each resource within a package must have a unique name.
	 */
	struct Package final : public std::enable_shared_from_this<Package> {
		struct PrivateToken;

		std::atomic<FPackageFlags> flags;

		Package() = delete;
		explicit Package(PrivateToken, std::shared_ptr<PackageDatabase> owner, StringID name) : owner(owner), name(name) {}

		/** Get the unique name of this package */
		inline StringID GetName() const { return name; }

		/** Returns true if this package contains a resource with the given name */
		bool Contains(StringID resourceName) const;
		/** Find a resource with the given name that is contained in this package */
		Handle<Resource> Find(StringID resourceName) const;
		
		/** Get a read-only thread-safe view that allows the contents of the package to be inspected */
		[[nodiscard]] inline auto GetContentsView() const { return thread.contents.LockInclusive(); }

		/** Add a new resource to this package. The resource must not already be contained in another package. */
		void Add(Reference<Resource> const& resource);
		/** Remove the resource from this package */
		void Remove(Reference<Resource> const& resource);

		/** Rename this package. Will throw if another package with this name already exists in the owning database.  */
		void Rename(StringID newName);

	private:
		friend Resource;
		friend PackageDatabase;

		struct PrivateToken {};

		std::weak_ptr<PackageDatabase> const owner;
		StringID name;

		struct {
			ThreadSafe<std::unordered_map<StringID, std::weak_ptr<Resource>>> contents;
		} thread;

		/** Mount a loaded resource to this package, associating it with the matching reference in the package. Called after loading a resource. */
		void Mount(Reference<Resource> const& resource);
		/** Unmount a loaded resource from this package, disassociating it with the matching reference in the package. Called before destroying a resource. */
		void Unmount(StringID resourceName);

		void RenameResource(Resource& resource, StringID newName);
	};
}

inline bool operator==(Resources::Package const& package, StringID name) { return package.GetName() == name; }
inline bool operator==(std::shared_ptr<Resources::Package const> const& package, StringID name) { return package && package->GetName() == name; }
