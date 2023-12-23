#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Resource.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	/** A handler for a certain type of resource, which can handle the details of how to create the resource. */
	template<typename ResourceType>
	struct ResourceCreationHandler {
		/** Construct a brand new instance of the resource at the specified memory location */
		virtual ResourceType& Construct(std::in_place_type_t<ResourceType>, std::byte* memory, Resources::Initializer& init) = 0;
		/** Load an instnace of the resource at the specified memory location using the buffer of serialized data */
		virtual ResourceType& Load(std::in_place_type_t<ResourceType>, std::byte* memory, std::span<std::byte const> buffer, Resources::Initializer& init) = 0;
		/** Called before the resource is removed from the cache */
		virtual void PreDestroy(ResourceType&) = 0;
	};
	
	struct ICache {
		virtual ~ICache() = default;
	};

	template<typename ResourceType>
	struct TrivialResourceCreationHandler : ResourceCreationHandler<ResourceType> {
		ResourceType& Construct(std::in_place_type_t<ResourceType>, std::byte* memory, Resources::Initializer& init) override {
			return *std::construct_at<ResourceType>(memory, init);
		}
		ResourceType& Load(std::in_place_type_t<ResourceType>, std::byte* memory, std::span<std::byte const> buffer, Resources::Initializer& init) override {
			return *std::construct_at<ResourceType>(memory, init);
		}
		void PreDestroy(ResourceType&) override {}
	};

	/** A cache that manages a specific type of resource */
	template<typename ResourceType, typename AllocatorType = std::allocator<ResourceType>>
	struct Cache : public ICache, public Handle<ResourceType>::Factory {
	public:
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
		virtual Handle<ResourceType> Create(StringID id) {
			std::unique_lock const lock{ mutex };

			//If we already have a resource with this id, it's an invalid operation to try to create another
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				LOG(Resources, Error, "Cannot create new resource with id {}. There is already a resource using this id.", id);
				return nullptr;
			}

			//Allocate the memory for the resource and associate it with the id. This allows the resource to be queried in the cache during its own constructor.
			ResourceType* resource = allocator.allocate(1);
			ids.emplace_back(id);
			resources.emplace_back(resource);

			Initializer const init{ id, resources.size() - 1 };
			std::construct_at(resource, init);
			
			Handle<ResourceType> const handle = CreateHandle(*resource, *resource);
			Created(handle);

			return handle;
		}

		virtual Handle<ResourceType> Load(StringID id, std::span<std::byte const> archive) { return nullptr; }

		virtual Handle<ResourceType> Find(StringID id) {
			std::shared_lock lock{ mutex };
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				ResourceType* resource = resources[iter - ids.begin()];
				return CreateHandle(*resource, *resource);
			}
			return nullptr;
		}

		virtual bool Contains(StringID id) {
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
		std::deque<StringID> ids;
		/** Resources that are currently in this cache */
		std::deque<ResourceType*> resources;
	};
}
