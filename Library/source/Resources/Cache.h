#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Resource.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	struct ICache {
		virtual ~ICache() = default;
	};

	/** A cache that manages a specific type of resource */
	template<typename ResourceType, typename AllocatorType = std::allocator<ResourceType>>
	struct Cache : public ICache, public Handle<ResourceType>::Factory {
	public:
		static_assert(std::is_base_of_v<Resource, ResourceType>, "Cache can only manage objects that derive from Resource");

		/** Broadcast just after a resource is created */
		TEvent<Handle<ResourceType> const&> Created;
		/** Broadcast just before a resource is destroyed */
		TEvent<Handle<ResourceType> const&> Destroyed;

		~Cache() {
			for (ResourceType* resource : resources) {
				std::destroy_at(resource);
				allocator.deallocate(resource, 1);
			}
		}

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
			ResourceType* resource = allocator.allocate(1);
			ids.emplace_back(id);
			resources.emplace_back(resource);

			Initializer init;
			init.id = id;
			init.index = resources.size() - 1;
			std::construct_at(resource, init);
			
			Handle<ResourceType> const handle = CreateHandle(*resource, *resource);
			Created(handle);

			return handle;
		}

		virtual Handle<ResourceType> Load(Identifier id, TArrayView<std::byte> archive) { return nullptr; }

		virtual Handle<ResourceType> Find(Identifier id) {
			std::shared_lock lock{ mutex };
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				ResourceType* resource = resources[iter - ids.begin()];
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
			for (ResourceType* resource : resources) {
				Destroyed(CreateHandle(*resource, *resource));
			}
		}

	protected:
		using Handle<ResourceType>::Factory::CreateHandle;

		/** Mutex which controls access to the resources in the database */
		stdext::shared_recursive_mutex mutex;
		/** Allocator used to allocate new resource objects */
		AllocatorType allocator;
		/** IDs for all resources in the database, indices are kept in sync with the resources collection */
		std::deque<Identifier> ids;
		/** Resources that are currently in this cache */
		std::deque<ResourceType*> resources;
	};
}
