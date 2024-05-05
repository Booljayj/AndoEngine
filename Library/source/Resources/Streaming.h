#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/StringID.h"
#include "Resources/Database.h"
#include "Resources/Package.h"

namespace Resources {
	/** A request to load a specific package. Used internally as part of the streaming process. */
	struct PackageRequest {
		using Result = std::expected<std::shared_ptr<Package>, std::string>;

		StringID const name;
		std::atomic<float> progress = 0.0f;
		TriggeredThreadSafe<std::optional<Result>> ts_result;

		PackageRequest(StringID name) : name(name) {}
		PackageRequest(stdext::shared_ref<Package> package) : name(package->GetName()), progress(1.0f), ts_result(package.get()) {}
	};

	/**
	 * A handle that allows external access to a particular package request.
	 * Handles can be copied or moved, and are equal if they both refer to the same requested package.
	 * They do not have a default state, a handle will always refer to a request that was made.
	 */
	struct PackageRequestHandle {
		PackageRequestHandle(stdext::shared_ref<PackageRequest> request) : request(request) {}
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
		stdext::shared_ref<PackageRequest> request;

		/** Helper that returns true if the result has a value assigned */
		static inline bool IsResultReady(std::optional<PackageRequest::Result> const& result) { return result.has_value(); }
	};

	/** The source data for a package, in binary format */
	struct PackageSource_Binary {
		/** The bytes that contain serialized data for the package */
		std::vector<std::byte> bytes;

		PackageSource_Binary(std::istream& stream);
		PackageSource_Binary(Package const& package);
	};
	/** The source data for a package, in YAML format */
	struct PackageSource_YAML {
		/** The root YAML node for the package */
		YAML::Node root;

		PackageSource_YAML(std::istream& stream);
		PackageSource_YAML(Package const& package);
	};

	/** The on-disk source for a package, in a specific known format. */
	using PackageSource = std::variant<PackageSource_Binary, PackageSource_YAML>;

	/** A database which supports streaming operations to load and save packages. Loading and saving is asynchronous. */
	struct StreamingDatabase : public Database {
		StreamingDatabase();
		~StreamingDatabase();

	protected:
		/** Save a known package. The location of the saved source and the format in which it is saved is determined by the implementer. */
		virtual bool SavePackage(Package const& package) = 0;
		/** Load the source for a known package. The location of the source and the format in which it is returned is determined by the implementer. */
		virtual PackageSource LoadPackageSource(StringID name) = 0;

		/** Save an existing package that has the provided name. If the package does not exist, an exception will be thrown. */
		bool SavePackage(StringID name);
		/** Load an existing package that has the provided name. If the package is already loaded, a handle to the loaded package will be returned instead. */
		PackageRequestHandle LoadPackage(StringID name);

	private:
		TriggeredThreadSafe<std::unordered_map<StringID, std::shared_ptr<PackageRequest>>> ts_requests;
		
		std::jthread thread;
		
		std::shared_ptr<Resource> CreateResource(StringID id, Reflection::StructTypeInfo const& type, absl::FunctionRef<void(Resource&)> initializer);
		
		void AsyncProcessRequests(std::stop_token token);
		std::shared_ptr<PackageRequest> AsyncGetHighestPriorityRequest(std::stop_token& token) const;
	};
}
