#pragma once
#include "Engine/Array.h"
#include "Engine/Events.h"
#include "Engine/Core.h"
#include "Engine/FunctionRef.h"
#include "Engine/Ranges.h"
#include "Engine/SmartPointers.h"
#include "Engine/Threads.h"
#include "Resources/Package.h"
#include "Resources/Resource.h"
#include "Resources/ResourceTypes.h"
#include "Resources/ResourceUtility.h"

namespace Resources {
	/**
	 * Base class for caches that manage a specific type of resource.
	 * Caches allow systems to easily and efficiently iterate over all resources of a given type. The order of iteration is unspecified.
	 * The resources within a cache are unnamed, identified only by their handles. A garbage collection pass can be performed to clean
	 * up resources that are no longer being used.
	 * Systems can also listen for events that are broadcast whenever a resource is created or destroyed, which allows them to perform
	 * bookkeeping related to the resource (such as uploading mesh data to a GPU for a static mesh resource).
	 */
	struct Cache : public std::enable_shared_from_this<Cache> {
		virtual ~Cache() = default;

		/**
		 * Destroy resources that are unused. If limit is not 0, this specifies some quantity that should not be exceeded during
		 * this call, possibly leaving unused resources in place if the limit is reached. The exact meaning of the limit is type-specific
		 * for the cache, but generally corresponds to the maximum number of individual resources that will be cleaned.
		 */
		virtual size_t CollectGarbage(size_t limit = std::numeric_limits<size_t>::max()) = 0;

		/** Create a resource, using the initializer function to assign values before notifying external systems about the new resource */
		virtual std::shared_ptr<Resource> Create(StringID name, FunctionRef<void(Resource&)> initializer) = 0;
	};

	/** An observer that can listen for when resources are created or destroyed by a specific cache */
	template<typename ResourceType>
	struct Observer {
		virtual void OnCreated(Handle<ResourceType> const& handle) = 0;
		virtual void OnDestroyed(Handle<ResourceType> const& handle) = 0;
	};

	/** A cache that manages a specific type of resource */
	template<typename ResourceType>
	struct TCache : public Cache {
	public:
		/** Add an observer that will be notified when resources are modified */
		void AddObserver(Observer<ResourceType>& observer) { observers.push_back(&observer); }
		/** Remove an observer that was previously added */
		void RemoveObserver(Observer<ResourceType>& observer) {
			auto const iter = ranges::find(observers, &observer);
			if (iter != observers.end()) observers.erase(iter);
		}

		virtual size_t CollectGarbage(size_t limit) override {
			auto resources = ts_resources.LockExclusive();

			size_t count = 0;

			//Pop unused back elements first, stopping if we exceed the limit.
			while (count < limit && resources->size() > 0 && resources->back().use_count() == 1) {
				NotifyDestroyed(resources->back());
				resources->pop_back();
				++count;
			}

			//If we still haven't reached the limit, move on to the other elements.
			if (count < limit) {

				//Iterate backwards through the container and perform a swap-and-pop on any unused elements.
				//Thanks to the loop above, we know that the last element is currently in-use, so we can start from the second-to-last element.
				//We can also iterate using one larger than the index to avoid underflow issues with the index.
				for (size_t rnum = resources->size() - 1; count < limit && rnum > 0; --rnum) {
					const size_t rindex = rnum - 1;

					auto& resource = resources[rindex];
					if (resource.use_count() == 1) {
						std::swap(resource, resources->back());
						NotifyDestroyed(resources->back());
						resources->pop_back();
						++count;
					}
				}
			}

			return count;
		}

		virtual std::shared_ptr<Resource> Create(StringID name, FunctionRef<void(Resource&)> initializer) override final {
			//Create the new resource object.
			std::shared_ptr<ResourceType> const resource = std::make_shared<ResourceType>(name);

			//Record the new resource entry. The amount of bookkeeping needed is minimal here, so we release the lock as soon as the information is recorded.
			//Releasing the lock also means the initializer can be recursive and potentially create other resources.
			{
				auto resources = ts_resources.LockExclusive();
				resources->emplace_back(resource);
			}

			//Call the initializer to load data into the resource
			initializer(*resource);
			//Broadcast to external systems that the resource has been created
			NotifyCreated(resource);

			return resource;
		}

		/** Perform an operation on all resources in this cache. Stops iterating when the operation returns false. */
		template<typename OperationType>
			requires std::is_invocable_r_v<bool, OperationType, ResourceType&>
		void ForEachResource(OperationType&& operation) const {
			auto const resources = ts_resources.LockInclusive();

			for (auto const& handle : *resources) {
				if (!operation(*handle)) break;
			}
		}

	protected:
		ThreadSafe<std::deque<std::shared_ptr<ResourceType>>> ts_resources;
		std::vector<Observer<ResourceType>*> observers;

		void NotifyCreated(std::shared_ptr<ResourceType> const& resource) {
			for (auto* observer : observers) { observer->OnCreated(resource); }
		}
		void NotifyDestroyed(std::shared_ptr<ResourceType> const& resource) {
			for (auto* observer : observers) { observer->OnDestroyed(resource); }
		}
	};
}
