#include "Resources/Streaming.h"

namespace Resources {
	bool PackageRequestHandle::HasFailed() const {
		auto const result = request->ts_result.LockInclusive();
		return result->has_value() && !result->value().has_value();
	}

	std::shared_ptr<Package> PackageRequestHandle::Get() const {
		auto const result = request->ts_result.LockInclusive();
		return result->has_value() ? result->value().value_or(nullptr) : nullptr;
	}

	std::shared_ptr<Package> PackageRequestHandle::Wait() {
		auto const result = request->ts_result.WaitInclusive(&IsResultReady);
		return result->value().value_or(nullptr);
	}

	std::shared_ptr<Package> PackageRequestHandle::Wait(std::chrono::high_resolution_clock::time_point time) {
		if (auto const possible = request->ts_result.WaitInclusive(time, &IsResultReady)) return (*possible)->value().value_or(nullptr);
		else return nullptr;
	}

	std::shared_ptr<Package> PackageRequestHandle::Wait(std::chrono::milliseconds duration) {
		if (auto const possible = request->ts_result.WaitInclusive(duration, &IsResultReady)) return (*possible)->value().value_or(nullptr);
		else return nullptr;
	}

	StreamingDatabase::StreamingDatabase() : async_request_thread(*this), thread(CreateThread(async_request_thread)) {}

	bool StreamingDatabase::SavePackage(StringID name) {
		std::shared_ptr<Package> const package = FindPackage(name);
		if (!package) throw FormatType<std::runtime_error>("Cannot find package {} when attempting to save", name);
		return SavePackage(*package);
	}

	PackageRequestHandle StreamingDatabase::LoadPackage(StringID name, RequestPriority priority) {
		return async_request_thread.CreateRequest(name, priority);
	}

	std::shared_ptr<Resource> StreamingDatabase::CreateResource(StringID id, Reflection::StructTypeInfo const& type, absl::FunctionRef<void(Resource&)> initializer) {
		if (!type.IsChildOf<Resource>()) {
			throw FormatType<std::runtime_error>("Type {} does not derive from Resource, cannot use this to create resource {}", type.name, id);
		}

		std::shared_ptr<Cache> const cache = FindOrCreateCache(type);
		return cache->Create(id, initializer);
	}

	std::unordered_map<StringID, std::shared_ptr<Resource>> StreamingDatabase::CreateContents(PackageInput_Binary& source) {
		using namespace Reflection;

		std::unordered_map<StringID, std::shared_ptr<Resource>> results;

		for (auto const [id, type_reference, buffer] : source.GetContentsInformation()) {
			if (auto const* type = type_reference.Resolve<StructTypeInfo>()) {
				auto const initialize = [&](Resource& resource) {
					Archive::Input archive{ buffer };
					type->Deserialize(archive, &resource);
				};

				if (auto const resource = CreateResource(id, *type, initialize)) {
					results.emplace(std::make_pair(id, resource));
				}
			}
		}

		return results;
	}

	std::unordered_map<StringID, std::shared_ptr<Resource>> StreamingDatabase::CreateContents(PackageInput_YAML& source) {
		using namespace Reflection;
		
		std::unordered_map<StringID, std::shared_ptr<Resource>> results;

		for (auto const [id, type_reference, object] : source.GetContentsInformation()) {
			if (auto const* type = type_reference.Resolve<Reflection::StructTypeInfo>()) {
				auto const initialize = [&](Resource& resource) { type->Deserialize(object, &resource); };

				if (auto const resource = CreateResource(id, *type, initialize)) {
					results.emplace(std::make_pair(id, resource));
				}
			}
		}

		return results;
	}

	StreamingDatabase::AsyncRequestThread::AsyncRequestThread(StreamingDatabase& database)
		: database(database)
	{}

	void StreamingDatabase::AsyncRequestThread::operator()(std::stop_token token) {
		struct DependencySearcher {
			/** Perform a depth-first search and return the first unloaded nested dependency of the provided package which can be loaded. Returns the provided package if all dependencies are loaded. */
			std::shared_ptr<PackageRequest> Search(std::shared_ptr<PackageRequest> const& current) {
				visited.clear();
				return SearchInternal(current);
			}
			
		private:
			std::unordered_set<PackageRequest const*> visited;

			std::shared_ptr<PackageRequest> SearchInternal(std::shared_ptr<PackageRequest> const& current) {
				for (std::shared_ptr<PackageRequest> const& shared_dependency : current->dependencies) {
					PackageRequest const& dependency = *shared_dependency;

					auto const result = visited.insert(&dependency);
					bool const was_visited = !result.second;

					if (!was_visited && dependency.IsPending()) {
						return SearchInternal(shared_dependency);
					}
				}

				//If none of the nested dependencies were unloaded, return this package itself.
				return current;
			}
		};

		DependencySearcher searcher;

		while (!token.stop_requested()) {
			//@todo Clean the requests, removing requests that no longer have any external references.
			//      This must be an interative process, because when we remove a request that may mean other requests now have no external references.

			if (std::shared_ptr<PackageRequest> const highest = GetHighestPriorityRequest(token)) {
				//@todo Check if the package is already canceled, and return early if it is.
				//      We should be periodically re-checking whether the package has been canceled, so we can stop and process another package instead.

				std::shared_ptr<PackageRequest> const current = searcher.Search(highest);

				try {
					if (!current->source.has_value()) {
						current->source.emplace(database.LoadPackageSource(current->name));

						const auto dependencies = std::visit([](auto& source) { return source.GetDependencies(); }, *current->source);
						current->dependencies = CreateDependencyRequests(dependencies);
						continue;

					} else {
						const auto contents = std::visit([this](auto& source) { return database.CreateContents(source); }, *current->source);

						std::shared_ptr<Package> const package = database.CreatePackageWithContents(current->name, contents);

						auto result = current->ts_result.LockExclusive();
						result->emplace(package);
					}
				} catch (std::exception const& e) {
					auto result = current->ts_result.LockExclusive();
					result->emplace(std::unexpected<std::string>(e.what()));
				}

				//Always notify listeners once the request is complete, even if it's a failure.
				current->ts_result.Notify();

				auto requests = ts_requests.LockExclusive();
				requests->erase(current->name);
			}
		}
	}

	PackageRequestHandle StreamingDatabase::AsyncRequestThread::CreateRequest(StringID name, RequestPriority priority) {
		auto requests = ts_requests.LockExclusive();

		//Attempt to find the package if it's already loaded.
		//We do this while the streaming is locked to avoid a race condition if multiple LoadPackage calls are made at the same time.
		{
			auto const packages = database.ts_packages.LockInclusive();
			auto const iter = packages->find(name);

			if (iter != packages->end()) {
				auto const request = std::make_shared<PackageRequest>(iter->second);
				return PackageRequestHandle{ request };
			}
		}

		//Attempt to find an existing streaming object for this package, and update the priority based on this new request
		auto const iter = requests->find(name);
		if (iter != requests->end()) {
			//@todo Increase the priority to the maximum value of all requests
			return PackageRequestHandle{ iter->second };

			//Create a new streaming object for this package, and notify waiting threads that a new streaming package was added.
		} else {
			auto const result = requests->emplace(std::make_pair(name, std::make_shared<PackageRequest>(name, priority)));
			ts_requests.Notify();
			return PackageRequestHandle{ result.first->second };
		}
	}

	std::shared_ptr<PackageRequest> StreamingDatabase::AsyncRequestThread::GetHighestPriorityRequest(std::stop_token& token) const {
		//We'll lock only as long as it takes to process the current streaming packages and choose one to update.
		auto const requests = ts_requests.WaitInclusive([&token](auto const& requests) { return requests.size() > 0 || token.stop_requested(); });

		//If we stopped waiting because of a shutdown, don't return a package.
		if (token.stop_requested() || requests->size() == 0) return nullptr;

		//@todo Clean the list of requests by removing requests that have no external references (ref_count is 1).

		//Now find the highest-priority request. This is the request that we should be trying to fulfill.
		RequestPriority highest_priority = 0;
		std::shared_ptr<PackageRequest> highest_request;

		auto iter = requests->begin();
		{
			highest_priority = iter->second->priority;
			highest_request = iter->second;
			++iter;
		}
		for (; iter != requests->end(); ++iter) {
			if (iter->second->priority > highest_priority) {
				highest_priority = iter->second->priority;
				highest_request = iter->second;
			}
		}

		return highest_request;
	}

	std::vector<std::shared_ptr<PackageRequest>> StreamingDatabase::AsyncRequestThread::CreateDependencyRequests(std::unordered_set<StringID> const& dependencies) {
		if (dependencies.size() > 0) {
			//Lock before iterating to make sure new requests cannot be filed while we are creating each dependency request.
			auto requests = ts_requests.LockExclusive();
			auto const packages = database.ts_packages.LockInclusive();

			std::vector<std::shared_ptr<PackageRequest>> results;
			ranges::transform(
				dependencies, std::back_inserter(results),
				[&requests, &packages](StringID name) {
					//Attempt to find the package if it's already loaded.
					{
						auto const iter = packages->find(name);

						if (iter != packages->end()) {
							return std::make_shared<PackageRequest>(iter->second);
						}
					}

					//Attempt to find an existing streaming object for this package
					auto const iter = requests->find(name);
					if (iter != requests->end()) {
						return iter->second;
					}

					//Create a new streaming object for this package.
					//We don't notify because this happens within the processing thread already, this can never wake the processing thread.
					auto const result = requests->emplace(std::make_pair(name, std::make_shared<PackageRequest>(name, LowestRequestPriority)));
					return result.first->second;
				}
			);
			return results;
		}

		return {};
	}
}
