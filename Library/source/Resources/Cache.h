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
	template<typename ResourceType>
	struct Cache : public ICache {
	public:
		/** Broadcast just after a resource is created */
		TEvent<Handle<ResourceType> const&> Created;
		/** Broadcast just before a resource is destroyed */
		TEvent<Handle<ResourceType> const&> Destroyed;

		/** Creates a new resource in the cache. Thread-safe. */
		virtual Handle<ResourceType> Create(StringID id) {
			std::unique_lock const lock{ mutex };

			//If we already have a resource with this id, it's an invalid operation to try to create another
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				LOG(Resources, Error, "Cannot create new resource with id {}. There is already a resource using this id.", id);
				return nullptr;
			}

			Initializer const init{ id, resources.size() };

			ids.emplace_back(id);
			auto const handle = resources.emplace_back(std::make_shared<ResourceType>(init));
			
			Created(handle);

			return handle;
		}

		virtual Handle<ResourceType> Load(StringID id, std::span<std::byte const> archive) { return nullptr; }

		virtual Handle<ResourceType> Find(StringID id) {
			std::shared_lock lock{ mutex };
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) return resources[std::distance(ids.begin(), iter)];
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
			for (auto const& resource : resources) {
				Destroyed(resource);
			}
		}

	protected:
		/** Mutex which controls access to the resources in the database */
		mutable stdext::recursive_shared_mutex mutex;
		/** IDs for all resources in the database, indices are kept in sync with the resources collection */
		std::deque<StringID> ids;
		/** Resources that are currently in this cache */
		std::deque<Handle<ResourceType>> resources;
	};
}
