#pragma once
#include "Engine/Events.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Engine/TupleUtility.h"
#include "Resources/Resource.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	struct ICache
	{
		virtual ~ICache() = default;
	};

	/** A cache that manages a specific type of resource */
	template<typename ResourceType>
	struct Cache : public ICache, public Handle<ResourceType>::Factory {
	public:
		static_assert(std::is_base_of_v<Resource, ResourceType>, "Cache can only manage objects that derive from Resource");

		TEvent<Handle<ResourceType> const&> Created;
		TEvent<Handle<ResourceType> const&> Destroyed;

		/** Creates a new resource in the cache. Thread-safe. */
		virtual Handle<ResourceType> Create(Identifier id) {
			std::unique_lock const lock{ mutex };

			//If we already have a resource with this id, it's an invalid operation to try to create another
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				LOGF(Resources, Error, "Cannot create new resource with id %s. There is already a resource using this id.", id);
				return nullptr;
			}

			//Allocate the memory for the resource and associate it with the id. This allows the resource to be queried in the cache during its own constructor.
			ids.emplace_back(id);
			ResourceStorage& storage = resources.emplace_back();

			ResourceType* resource = storage.Get();
			new (resource) ResourceType(id);

			Handle<ResourceType> const handle = CreateHandle(*resource, *resource);
			Created(handle);

			return handle;
		}

		virtual Handle<ResourceType> Load(Identifier id, TArrayView<std::byte> archive) { return nullptr; }

		virtual Handle<ResourceType> Find(Identifier id) {
			std::shared_lock lock{ mutex };
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				ResourceType* resource = resources[iter - ids.begin()].Get();
				return CreateHandle(*resource, *resource);
			}
			return nullptr;
		}

		virtual bool Contains(Identifier id) {
			std::shared_lock lock{ mutex };
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) return true;
			else return false;
		}

		/** Destroys any registered state for all resources. The resources will still exist, but they should be ready to be deallocated. */
		void Destroy() {
			std::shared_lock lock{ mutex };
			for (ResourceStorage& storage : resources) {
				ResourceType* resource = storage.Get();
				Destroyed(CreateHandle(*resource, *resource));
			}
		}

	protected:
		struct ResourceStorage {
			alignas(ResourceType) std::byte bytes[sizeof(ResourceType)];
			ResourceType* Get() { return reinterpret_cast<ResourceType*>(&bytes); }
		};

		using Handle<ResourceType>::Factory::CreateHandle;

		/** Mutex which controls access to the resources in the database */
		stdext::shared_recursive_mutex mutex;
		/** IDs for all resources in the database, indices are kept in sync with the resources collection */
		std::deque<Identifier> ids;
		/** Resources that are currently in this cache */
		std::deque<ResourceStorage> resources;
	};

	/**
	 * Databases keep track of a collection of resources, and have utility methods to create, load, and find them.
	 * Some resources are predefined and will alawys have caches with inline accessors. Others are created
	 * on-demand and accessed in O(1) time.
	 */
	template<typename... ResourceTypes>
	struct Database {
	public:
		template<typename ResourceType>
		Cache<ResourceType>& GetCache() {
			using Tuple = std::tuple<ResourceTypes...>;
			constexpr size_t TypeIndex = TupleUtility::Index<ResourceType, Tuple>();
			if constexpr (TypeIndex < std::tuple_size_v<Tuple>) {
				return std::get<TypeIndex>(defined_caches);

			} else {
				using CacheType = Cache<ResourceType>;
				KeyType key = GetKey<ResourceType>();
				CacheMap::const_iterator const iter = generic_caches.find(key);
				if (iter != generic_caches.end()) return static_cast<CacheType*>(iter->second);
				else return static_cast<CacheType*>(generic_caches.emplace(std::make_pair(key, new CacheType())).first);
			}
		}

	protected:
		using CacheTuple = std::tuple<Cache<ResourceTypes>...>;
		using KeyType = uint8_t const*;
		using CacheMap = std::unordered_map<KeyType, ICache*>;

		CacheTuple defined_caches;
		CacheMap generic_caches;

		/** Generates a runtime unique key for a type */
		template<typename ResourceType>
		static KeyType GetKey() {
			static const uint8_t key = 0b01010101;
			return &key;
		}
	};
}
