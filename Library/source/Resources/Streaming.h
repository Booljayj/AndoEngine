#pragma once
#include "Engine/Core.h"
#include "Engine/Map.h"
#include "Engine/Optional.h"
#include "Engine/SmartPointers.h"
#include "Engine/StringID.h"
#include "Engine/Threads.h"
#include "Engine/Vector.h"
#include "Resources/Database.h"
#include "Resources/Package.h"
#include "Resources/PackageIO.h"

namespace Resources {
	using RequestPriority = uint16_t;
	constexpr RequestPriority DefaultRequestPriority = 1000;
	constexpr RequestPriority LowestRequestPriority = 0;
	constexpr RequestPriority HighestRequestPriority = std::numeric_limits<RequestPriority>::max();

	/** A request to load a specific package. Used internally as part of the streaming process. */
	struct PackageRequest {
		using Result = std::expected<std::shared_ptr<Package>, std::string>;
		
		/** The name of the package being requested. Cannot change after the request is made, each request may only refer to a single package. */
		StringID const name;

		/** The priority at which the package is being requested. The priority can increase if a higher-priority request is made, but cannot decrease. */
		std::atomic<RequestPriority> priority = DefaultRequestPriority;
		/** An approximation of the progress of loading this package. Does not include the progress of loading dependencies. */
		std::atomic<float> progress = 0.0f;

		/** The source information being read to create the package. Available once the package has started the process of loading. */
		std::optional<PackageInput> source;
		/** The set of first-level dependencies that must be loaded before this package can be loaded */
		std::vector<std::shared_ptr<PackageRequest>> dependencies;
		/** The final result of this request, which is created only when it is finished. Some requests are created in an already-finished state, and this will be immediately available. */
		TriggeredThreadSafe<std::optional<Result>> ts_result;

		PackageRequest(StringID name, RequestPriority priority) : name(name), priority(priority) {}
		PackageRequest(std::shared_ptr<Package> package) : name(package->GetName()), progress(1.0f), ts_result(package.get()) {}

		/** True if this request is still pending and does not have a result yet */
		inline bool IsPending() const { return !ts_result.LockInclusive()->has_value(); }
	};

	/**
	 * A handle that allows external access to a particular package request.
	 * Handles can be copied or moved, and are equal if they both refer to the same requested package.
	 * They do not have a default state, a handle instance will always refer to a request that was made.
	 */
	struct PackageRequestHandle {
		PackageRequestHandle(std::shared_ptr<PackageRequest> request) : request(request) {}
		PackageRequestHandle(PackageRequestHandle const&) = default;
		PackageRequestHandle(PackageRequestHandle&&) = default;

		PackageRequestHandle& operator=(PackageRequestHandle const&) = default;
		PackageRequestHandle& operator=(PackageRequestHandle&&) = default;

		inline bool operator==(PackageRequestHandle const& other) const { return request->name == other.request->name; }

		/** Get the name of the package that this streaming object is loading */
		inline StringID GetName() const { return request->name; }
		/** Get the progress of loading this package expressed as a ratio between 0 and 1 */
		inline float GetProgress() const { return request->progress; }

		/** True if the streaming failed at some point for the package, meaning this streaming object will no longer result in a final package. */
		bool HasFailed() const;
		/** Get the package if it is already loaded. Will return nullptr if the package is not loaded, or if streaming did not finish. */
		std::shared_ptr<Package> Get() const;

		/** Block and wait until the package is finished loading, then return the result */
		std::shared_ptr<Package> Wait();
		/** Block and wait until the package is finished loading or the specified time, then return the result */
		std::shared_ptr<Package> Wait(std::chrono::high_resolution_clock::time_point time);
		/** Block and wait until the package is finished loading or the duration has elapsed, then return the result */
		std::shared_ptr<Package> Wait(std::chrono::milliseconds duration);

	private:
		std::shared_ptr<PackageRequest> request;

		/** Helper that returns true if the result has a value assigned */
		static inline bool IsResultReady(std::optional<PackageRequest::Result> const& result) { return result.has_value(); }
	};

	/** A database which supports streaming operations to load and save packages. Loading and saving is asynchronous. */
	struct StreamingDatabase : public Database {
		StreamingDatabase();

	protected:
		/** Save a known package. The location of the saved source and the format in which it is saved is determined by the implementer. */
		virtual bool SavePackage(Package const& package) = 0;
		/** Load the source for a known package. The location of the source and the format in which it is returned is determined by the implementer. */
		virtual PackageInput LoadPackageSource(StringID name) = 0;

		/** Save an existing package that has the provided name. If the package does not exist, an exception will be thrown. */
		bool SavePackage(StringID name);
		/** Load an existing package that has the provided name. If the package is already loaded, a handle to the loaded package will be returned instead. */
		PackageRequestHandle LoadPackage(StringID name, RequestPriority priority = DefaultRequestPriority);

	private:
		struct AsyncRequestThread {
			AsyncRequestThread(StreamingDatabase& database);

			void operator()(std::stop_token token);

			PackageRequestHandle CreateRequest(StringID name, RequestPriority priority);

		private:
			using ThreadSafeRequestsContainer = TriggeredThreadSafe<std::unordered_map<StringID, std::shared_ptr<PackageRequest>>>;

			StreamingDatabase& database;
			ThreadSafeRequestsContainer ts_requests;

			std::shared_ptr<PackageRequest> GetHighestPriorityRequest(std::stop_token& token) const;
			std::vector<std::shared_ptr<PackageRequest>> CreateDependencyRequests(std::unordered_set<StringID> const& dependencies);
		};

		AsyncRequestThread async_request_thread;
		std::jthread thread;

		std::shared_ptr<Resource> CreateResource(StringID id, Reflection::StructTypeInfo const& type, absl::FunctionRef<void(Resource&)> initializer);
		std::unordered_map<StringID, std::shared_ptr<Resource>> CreateContents(PackageInput_Binary& source);
		std::unordered_map<StringID, std::shared_ptr<Resource>> CreateContents(PackageInput_YAML& source);
	};
}
