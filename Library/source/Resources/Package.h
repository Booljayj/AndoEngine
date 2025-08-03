#pragma once
#include "Engine/Reflection.h"
#include "Engine/Core.h"
#include "Engine/Flags.h"
#include "Engine/SmartPointers.h"
#include "Engine/StringID.h"
#include "Engine/Threads.h"
#include "Resources/Resource.h"

namespace Resources {
	struct Database;

	/** Flags that describe various states or metadata that apply to an individual package. Some flags are only used in specific contexts. */
	enum class EPackageFlags {
		/** This package has pending changes that should be saved */
		Dirty,
		/** This package was loaded from disk, rather than being newly created */
		Loaded,
	};
	using FPackageFlags = TAtomicFlags<EPackageFlags>;

	/**
	 * A package is an abstract representation of a collection of resources.
	 * It may correspond to a file, or to a location within an archive, or something else depending on the implementation.
	 * Each package must have a unique name, and each resource within a package must have a unique name.
	 */
	struct Package final : public std::enable_shared_from_this<Package> {
		using ContentsContainerType = std::unordered_map<StringID, std::shared_ptr<Resource>>;

		FPackageFlags flags;

		Package(std::shared_ptr<Database> owner, StringID name) : owner(owner), name(name) {}
		Package(std::shared_ptr<Database> owner, StringID name, ContentsContainerType const& contents);

		/** Get the unique name of this package */
		inline StringID GetName() const { return name; }
		
		/** Return the resource with the provided id that is within this package, or nullptr if it cannot be found */
		template<std::derived_from<Resource> T = Resource>
		std::shared_ptr<T> Find(StringID id) const {
			auto const contents = ts_contents.LockInclusive();
			auto const iter = contents->find(id);
			if (iter != contents->end()) {
				if constexpr (std::is_same_v<T, Resource>) {
					return iter->second;
				} else {
					if (iter->second->GetTypeInfo().IsChildOf(Reflect<T>::Get())) return std::static_pointer_cast<T>(iter->second);
				}
			}
			return nullptr;
		}

		/** Get a read-only thread-safe view that allows the contents of the package to be inspected. Used to optimize bulk search operations on a package's contents. */
		[[nodiscard]] inline auto GetContentsView() const { return ts_contents.LockInclusive(); }

	private:
		friend struct Database;
		friend struct Resource;
		friend struct ResourceUtility;

		std::weak_ptr<Database> const owner;
		StringID name;
		ThreadSafe<ContentsContainerType> ts_contents;
	};
}

inline bool operator==(Resources::Package const& package, StringID name) { return package.GetName() == name; }
inline bool operator==(std::shared_ptr<Resources::Package const> const& package, StringID name) { return package && package->GetName() == name; }
