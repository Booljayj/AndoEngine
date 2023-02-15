#pragma once
#include "Engine/Events.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Resources/Manifest.h"
#include "Resources/Resource.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	/** Databases keep track of a collection of resources, and have utility methods to create, load, and find them. */
	struct Database {
	public:
		/** Creates a new resource in the database. Thread-safe. */
		virtual Handle<Resource> Create(Identifier id, Reflection::StructTypeInfo const& type) = 0;
		virtual Handle<Resource> Load(Identifier id, Reflection::StructTypeInfo const& type) = 0;
		virtual Handle<Resource> Find(Identifier id, Reflection::StructTypeInfo const& type) = 0;
		virtual bool Contains(Identifier id, Reflection::StructTypeInfo const& type) = 0;

		const IManifest& GetManifest() const { return manifest; }

	protected:
		/** IDs for all resources in the database, indices are kept in sync with the resources collection */
		std::deque<Identifier> ids;
		/** Mutex which controls access to the resources in the database */
		stdext::shared_recursive_mutex mutex;
		/** The manifest which determines where known resources can be found */
		IManifest const& manifest;

		Database(IManifest const& inManifest) : manifest(inManifest) {}
		~Database() = default;
	};

	/** Implements behavior for storing and tracking a specific type of resource */
	template<typename BaseResourceType, typename Implementation>
	struct TSparseDatabase : public Database, public ManagedObject::Handle<BaseResourceType>::Factory {
	public:
		static_assert(std::is_base_of_v<Resource, BaseResourceType>, "BaseResourceType must inherit from Resource");

		TEvent<Handle<BaseResourceType> const&> Created;
		TEvent<Handle<BaseResourceType> const&> Destroyed;

		TSparseDatabase(IManifest const& inManifest) : Database(inManifest) {}
		~TSparseDatabase() = default;

		virtual Handle<Resource> Create(Identifier id, Reflection::StructTypeInfo const& type) override {
			//Ensure the resource we are trying to create is a type that this database expects to manage
			if (!Reflection::StructTypeInfo::IsDerivedFrom(*ReflectStruct<BaseResourceType>::Get(), type)) {
				LOGF(Resources, Error, "Cannot create new resource with id %s. The desired type does not derive from %s", id);
				return nullptr;
			}

			//Ensure the id is not already used by a predefined resource
			if (manifest.GetInfo(id)) {
				LOGF(Resources, Error, "Cannot create new resource with id %s. There is already a resource using this id.", id);
				return nullptr;
			}

			std::unique_lock const lock{ mutex };

			//If we already have a resource with this id, it's an invalid operation to try to create another
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				LOGF(Resources, Error, "Cannot create new resource with id %s. There is already a resource using this id.", id);
				return nullptr;
			}

			ids.emplace_back(id);

			//Create the memory in the array for the resource, store it, then construct the value. This allows constructors to access their own entry if necessary.
			Reflection::TypeUniquePointer& pointer = resources.emplace_back(type.Allocate());
			BaseResourceType* resource = reinterpret_cast<BaseResourceType*>(pointer.get());

			type.Construct(resource);
			resource->id = id;
			static_cast<Implementation*>(this)->PostCreate(*resource);

			Handle<BaseResourceType> const handle = CreateHandle(*resource, *resource);
			Created(handle);

			return Cast<Resource>(std::move(handle));
		}

		//@todo Implement loading behavior
		virtual Handle<Resource> Load(Identifier id, Reflection::StructTypeInfo const& type) override { return nullptr; }

		virtual Handle<Resource> Find(Identifier id, Reflection::StructTypeInfo const& type) override {
			std::shared_lock lock{ mutex };
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				BaseResourceType* resource = reinterpret_cast<BaseResourceType*>(resources[iter - ids.begin()].get());
				if (Reflection::StructTypeInfo::IsDerivedFrom(type, resource->GetTypeInfo())) {
					return Cast<Resource>(CreateHandle(*resource, *resource));
				}
			}
			return nullptr;
		}

		virtual bool Contains(Identifier id, Reflection::StructTypeInfo const& type) override {
			std::shared_lock lock{ mutex };
			auto const iter = std::find(ids.begin(), ids.end(), id);
			if (iter != ids.end()) {
				Resource* resource = reinterpret_cast<Resource*>(resources[iter - ids.begin()].get());
				if (Reflection::StructTypeInfo::IsDerivedFrom(type, resource->GetTypeInfo())) {
					return true;
				}
			}
			return false;
		}

		/** Create a new resource with the given id. The id of the new resource must be unique. */
		template<typename ResourceType = BaseResourceType>
		Handle<ResourceType> Create(Identifier id) {
			static_assert(std::is_base_of_v<BaseResourceType, ResourceType>, "ResourceType must inherit from BaseResourceType");
			return Cast<ResourceType>(Create(id, *Reflect<ResourceType>::Get()));
		}

		template<typename ResourceType = BaseResourceType>
		Handle<ResourceType> Load(Identifier id) {
			static_assert(std::is_base_of_v<BaseResourceType, ResourceType>, "ResourceType must inherit from BaseResourceType");
			return Cast<ResourceType>(Load(id, *Reflect<ResourceType>::Get()));
		}

		template<typename ResourceType = BaseResourceType>
		Handle<ResourceType> Find(Identifier id) {
			static_assert(std::is_base_of_v<BaseResourceType, ResourceType>, "ResourceType must inherit from BaseResourceType");
			return Cast<ResourceType>(Find(id, *Reflect<ResourceType>::Get()));
		}

		/** Destroys any registered state for all resources. The resources will still exist, but they should be ready to be deallocated. */
		void Destroy() {
			std::shared_lock lock{ mutex };
			for (Reflection::TypeUniquePointer& pointer : resources) {
				BaseResourceType* resource = reinterpret_cast<BaseResourceType*>(pointer.get());
				Destroyed(CreateHandle(*resource, *resource));
			}
		}

	protected:
		using ManagedObject::Handle<BaseResourceType>::Factory::CreateHandle;

		/** Pointers to resources that are currently in this database */
		std::deque<Reflection::TypeUniquePointer> resources;
	};
}
