#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/StringID.h"
#include "Engine/Threads.h"
#include "Resources/Cache.h"
#include "Resources/Resource.h"

namespace Resources {
	struct Package;
	
	/** Base class for databases, which manage a set of packages and their resources. Protected methods can be exposed by derived type as needed. */
	struct Database : std::enable_shared_from_this<Database> {
	public:
		/** Get the global transient package, which is named "None". */
		static std::shared_ptr<Package> const& GetTransient() { return transient; }

		/** Returns true if the database contains an existing package with the provided name */
		bool ContainsPackage(StringID name) const;

		/** Find a specific named resource of any type using an identifier */
		std::shared_ptr<Resource> FindResource(Identifier id) const;

		/** Find a specific named resource of the provided type using an identifier */
		template<Concepts::DerivedFromResource T>
		std::shared_ptr<T> FindResource(Identifier id) const {
			if constexpr (std::is_same_v<T, Resource>) return FindResource(id);
			else {
				std::shared_ptr<Resource> const resource = FindResource(id, Reflect<T>::Get());
				if (resource && resource->GetTypeInfo().IsChildOf<T>()) {
					return std::static_pointer_cast<T>(resource);
				}
				return nullptr;
			}
		}

		/** Find the cache that stores all resources of the provided type. Returns none if the cache doesn't already exist. */
		template<Concepts::DerivedFromResource T>
		std::shared_ptr<TCache<T>> FindCache() const {
			auto const caches = ts_caches.LockInclusive();

			auto const iter = caches->find(Reflect<T>::ID);
			if (iter != caches->end()) return std::static_pointer_cast<TCache<T>>(iter->second);
			else return nullptr;
		}

		/** Find the cache that stores all resources of the provided type. Creates the cache if it doesn't already exist. */
		template<Concepts::DerivedFromResource T>
		stdext::shared_ref<TCache<T>> FindOrCreateCache() {
			auto caches = ts_caches.LockExclusive();

			auto const iter = caches->find(Reflect<T>::ID);
			if (iter != caches->end()) return std::static_pointer_cast<TCache<T>>(iter->second);

			auto const cache = std::make_shared<TCache<T>>();
			caches->emplace(Reflect<T>::ID, cache);
			return cache;
		}

	protected:
		/** Find an existing package by name */
		std::shared_ptr<Package> FindPackage(StringID name) const;
		/** Create a new empty package with the provided name. If a package with this name already exists, that will be returned instead. */
		std::shared_ptr<Package> CreatePackage(StringID name);
		/** Load an existing package that has the provided name. If the package is already loaded, it will be returned instead. */
		std::future<std::shared_ptr<Package>> LoadPackage(StringID name);

		/** Destroy an existing package by name. Will throw if the package cannot be destroyed because it is being used */
		void DestroyPackage(StringID name);

		/** Create a new empty resource of the specified type in the provided package. Will throw if the resource already exists or cannot be created. */
		template<Concepts::DerivedFromResource T, std::invocable<T&> InitializerType>
		std::shared_ptr<T> Create(StringID name, stdext::shared_ref<Package> package, InitializerType&& initializer) {
			stdext::shared_ref<TCache<T>> const cache = FindOrCreateCache<T>();
			return cache->Create(name, package, std::forward<InitializerType>(initializer));
		}

	private:
		friend Package;
		friend ResourceUtility;

		//The global transient package, which contains resources that don't belong to another package.
		static std::shared_ptr<Package> const transient;

		ThreadSafe<std::unordered_map<StringID, std::shared_ptr<Package>>> ts_packages;
		ThreadSafe<std::unordered_map<Hash128, std::shared_ptr<Cache>>> ts_caches;
	};
}
