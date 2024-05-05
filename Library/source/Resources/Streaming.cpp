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

	PackageSource_Binary::PackageSource_Binary(std::istream& stream) {
		std::for_each(
			std::istream_iterator<char>{ stream }, std::istream_iterator<char>{},
			[this](char c) { bytes.push_back(std::byte{ static_cast<unsigned char>(c) }); }
		);
	}

	PackageSource_Binary::PackageSource_Binary(Package const& package) {
		Archive::Output archive{ bytes };

		auto const contents = package.GetContentsView();
		archive << contents->size();

		std::vector<std::byte> resource_bytes;
		for (auto const& pair : *contents) {
			StringID const name = pair.first;
			Resources::Resource const& resource = *pair.second;

			Reflection::StructTypeInfo const& type = resource.GetTypeInfo();
			{
				//@todo We don't need a temporary vector here if we can write a "section" to the output, where the size is written before the bytes following it.
				resource_bytes.clear();
				Archive::Output resource_archive{ resource_bytes };
				type.Serialize(resource_archive, &resource);
			}

			archive << name << Reflection::TypeInfoReference{ type } << resource_bytes;
		}
	}

	PackageSource_YAML::PackageSource_YAML(std::istream& stream)
		: root(YAML::Load(stream))
	{}

	PackageSource_YAML::PackageSource_YAML(Package const& package)
		: root(YAML::NodeType::Map)
	{
		YAML::Node resources{ YAML::NodeType::Map };

		//Lock the contents only as long as it takes to serialize the contents. After the lock is released, further changes to the package are not saved.
		auto const contents = package.GetContentsView();
		for (auto const& pair : *contents) {
			StringID const name = pair.first;
			Resources::Resource const& resource = *pair.second;
			Reflection::StructTypeInfo const& type = resource.GetTypeInfo();

			YAML::Node node{ YAML::NodeType::Map };
			node["type"] = Reflection::TypeInfoReference{ type };
			node["object"] = type.Serialize(&resource);

			resources[name.ToStringView()] = node;
		}

		root["resources"] = resources;
	}

	StreamingDatabase::StreamingDatabase() : thread(std::bind_front(&StreamingDatabase::AsyncProcessRequests, this)) {}

	StreamingDatabase::~StreamingDatabase() {
		thread.request_stop();
		ts_requests.Notify();
	}

	bool StreamingDatabase::SavePackage(StringID name) {
		std::shared_ptr<Package> const package = FindPackage(name);
		if (!package) throw FormatType<std::runtime_error>("Cannot find package {} when attempting to save", name);
		return SavePackage(*package);
	}

	PackageRequestHandle StreamingDatabase::LoadPackage(StringID name) {
		auto requests = ts_requests.LockExclusive();

		//Attempt to find the package if it's already loaded.
		//We do this while the streaming is locked to avoid a race condition if multiple LoadPackage calls are made at the same time.
		{
			auto const packages = ts_packages.LockInclusive();
			auto const iter = packages->find(name);

			if (iter != packages->end()) {
				auto const request = std::make_shared<PackageRequest>(iter->second);
				return PackageRequestHandle{ request };
			}
		}

		//Attempt to find an existing streaming object for this package
		auto const iter = requests->find(name);
		if (iter != requests->end()) {
			return PackageRequestHandle{ iter->second };

			//Create a new streaming object for this package, and notify waiting threads that a new streaming package was added.
		} else {
			auto const result = requests->emplace(std::make_pair(name, std::make_shared<PackageRequest>(name)));
			ts_requests.Notify();
			return PackageRequestHandle{ result.first->second };
		}
	}

	std::shared_ptr<Resource> StreamingDatabase::CreateResource(StringID id, Reflection::StructTypeInfo const& type, absl::FunctionRef<void(Resource&)> initializer) {
		if (!type.IsChildOf<Resource>()) {
			throw FormatType<std::runtime_error>("Type {} does not derive from Resource, cannot use this to create resource {}", type.name, id);
		}

		std::shared_ptr<Cache> const cache = FindOrCreateCache(type);
		return cache->Create(id, initializer);
	}

	void StreamingDatabase::AsyncProcessRequests(std::stop_token token) {
		while (!token.stop_requested()) {
			if (std::shared_ptr<PackageRequest> const current = AsyncGetHighestPriorityRequest(token)) {
				//@todo Check if the package is already canceled, and return early if it is.
				//      We need to check before loading the source, before creating the package, and before assigning the result.

				try {
					PackageSource const source = LoadPackageSource(current->name);

					struct DeserializeVisitor {
						std::unordered_map<StringID, std::shared_ptr<Resource>> contents;

						DeserializeVisitor(StringID name, StreamingDatabase& owner) : name(name), owner(owner) {}

						void operator()(PackageSource_Binary const& source) {
							Archive::Input archive{ source.bytes };

							size_t count = 0;
							archive >> count;

							contents.reserve(count);

							for (size_t index = 0; index < count; ++index) {
								StringID id = StringID::None;
								Reflection::TypeInfoReference type_reference;
								std::span<std::byte const> buffer;

								archive >> id >> type_reference >> buffer;

								//We've read everything from the archive for this entry, now attempt to create the package
								if (auto const* type = type_reference.Resolve<Reflection::StructTypeInfo>()) {
									auto const initialize = [&](Resource& resource) {
										Archive::Input archive{ buffer };
										type->Deserialize(archive, &resource);
									};

									if (auto const resource = owner.CreateResource(id, *type, initialize)) {
										contents.emplace(std::make_pair(id, resource));
									}
								}
							}
						}

						void operator()(PackageSource_YAML const& source) {
							YAML::Node const resources = source.root["resources"];

							contents.reserve(resources.size());

							for (YAML::const_iterator iter = resources.begin(); iter != resources.end(); ++iter) {
								StringID const id = iter->first.as<StringID>();
								Reflection::TypeInfoReference const type_reference = iter->second["type"].as<Reflection::TypeInfoReference>();
								YAML::Node const object = iter->second["object"];

								//We've read everything from the archive for this entry, now attempt to create the package
								if (auto const* type = type_reference.Resolve<Reflection::StructTypeInfo>()) {
									auto const initialize = [&](Resource& resource) { type->Deserialize(object, &resource); };

									if (auto const resource = owner.CreateResource(id, *type, initialize)) {
										contents.emplace(std::make_pair(id, resource));
									}
								}
							}
						}

					private:
						StringID name;
						StreamingDatabase& owner;
					};

					DeserializeVisitor visitor{ current->name, *this };
					std::visit(visitor, source);

					std::shared_ptr<Package> const package = CreatePackageWithContents(current->name, visitor.contents);

					auto result = current->ts_result.LockExclusive();
					result->emplace(package);
				}
				catch (std::exception const& e) {
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

	std::shared_ptr<PackageRequest> StreamingDatabase::AsyncGetHighestPriorityRequest(std::stop_token& token) const {
		//We'll lock only as long as it takes to sort the current streaming packages and choose one to update.
		auto const requests = ts_requests.WaitInclusive([&token](auto const& requests) { return requests.size() > 0 || token.stop_requested(); });

		//If we stopped waiting because of a shutdown, don't return a package.
		if (token.stop_requested()) return nullptr;

		//@todo This should return the highest-priority instance, based on some understanding of priority.
		for (auto const& pair : *requests) return pair.second;
		return nullptr;
	}
}
