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
		static std::shared_ptr<Package> const& GetTransient() noexcept { return transient; }

		/** Returns true if the database contains an existing package with the provided name */
		bool ContainsPackage(StringID name) const noexcept;

		/** Find a specific named resource of any type using an identifier */
		std::shared_ptr<Resource> FindResource(Identifier id) const noexcept;

		/** Find a specific named resource of the provided type using an identifier */
		template<Concepts::DerivedFromResource T>
		std::shared_ptr<T> FindResource(Identifier id) const noexcept {
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
		std::shared_ptr<TCache<T>> FindCache() const noexcept {
			return std::static_pointer_cast<TCache<T>>(FindCache(Reflect<T>::Get()));
		}

		/** Find the cache that stores all resources of the provided type. Creates the cache if it doesn't already exist. */
		template<Concepts::DerivedFromResource T>
		stdext::shared_ref<TCache<T>> FindOrCreateCache() noexcept {
			stdext::shared_ref<Cache> const cache = FindOrCreateCache(Reflect<T>::Get());
			return std::static_pointer_cast<TCache<T>>(cache.get());
		}

	protected:
		friend Package;
		friend ResourceUtility;

		//The global transient package, which contains resources that don't belong to another package and will not be ever saved to disk.
		static std::shared_ptr<Package> const transient;

		ThreadSafe<std::unordered_map<StringID, std::shared_ptr<Package>>> ts_packages;
		ThreadSafe<std::unordered_map<Hash128, std::shared_ptr<Cache>>> ts_caches;

		/** Find an existing package by name */
		std::shared_ptr<Package> FindPackage(StringID name) const noexcept;
		/** Create a new empty package with the provided name. If a package with this name already exists, that will be returned instead. */
		std::shared_ptr<Package> CreatePackage(StringID name);
		/** Find an existing package by name if it already exists, or create a new package if it doesn't already exists. If the package exists but is not loaded, the package will not be loaded and this will throw and exception. */
		std::shared_ptr<Package> FindOrCreatePackage(StringID name);

		/** Create a new package with the provided name and contents. The contents must not already belong to another package, and the package name must not already exist. */
		std::shared_ptr<Package> CreatePackageWithContents(StringID name, std::unordered_map<StringID, std::shared_ptr<Resource>> const& contents);

		/** Destroy an existing package by name. Will throw if the package cannot be destroyed because it is being used */
		void DestroyPackage(StringID name);

		/** Returns whether it's possible to create a new package with the given name, assuming it's not already created. */
		virtual bool CanCreatePackage(StringID name) const { return false; }

		/** Find an existing generic cache using a type */
		std::shared_ptr<Cache> FindCache(Reflection::StructTypeInfo const& type);
		/** Find or create a generic cache using a type */
		stdext::shared_ref<Cache> FindOrCreateCache(Reflection::StructTypeInfo const& type);
		
		/** Create a new empty resource of the specified type in the provided package. Will throw if the resource already exists or cannot be created. */
		template<Concepts::DerivedFromResource T, std::invocable<T&> InitializerType>
		std::shared_ptr<T> Create(StringID name, stdext::shared_ref<Package> package, InitializerType&& initializer) {
			stdext::shared_ref<TCache<T>> const cache = FindOrCreateCache<T>();

			const auto wrapped_initializer = [&initializer, &package](Resource& resource) {
				initializer(static_cast<T&>(resource));
				ResourceUtility::MoveResource(resource.shared_from_this(), package);
			};
			return std::static_pointer_cast<T>(cache->Create(name, wrapped_initializer));
		}

	};
}
