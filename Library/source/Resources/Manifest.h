#pragma once
#include "Engine/STL.h"
#include "Resources/ResourceTypes.h"

//EXPERIMENTAL, UNDER CONSTRUCTION

namespace Resources {
	/** Persistent info about a resource which can be queried without loading the real resource */
	struct PersistentInfo {
		/** The human-readable name of the resource */
		std::string name;
		/** Arbitrary flags for this resource */
		uint64_t flags;
		/** Metadata values for this resource */
		std::unordered_map<Hash32, std::string> metadata;
	};

	/** The raw on-disk data for a resource */
	struct IResourceBuffer {
		/** Load the buffer for the resource and store it in the provided vector */
		virtual void Load(std::vector<uint8_t>& buffer) = 0;
		/** Save the provided vector into the buffer for the resource */
		virtual void Save(std::vector<uint8_t> const& buffer) = 0;
	};

	/** Contains information about where on disk to load the data for known resources */
	struct IManifest {
		/** Returns the persistent info for a given resource. If a resource can be loaded, this info will exist. */
		virtual PersistentInfo const* GetInfo(Identifier id) const = 0;
		/** Open the file that contains the data for the provided resource, and return a handle used to read from it */
		virtual std::shared_ptr<IResourceBuffer> Open(Identifier id) = 0;
	};

	/** Dummy manifest which cannot load any resources */
	struct DummyResourceManifest : public IManifest {
		virtual PersistentInfo const* GetInfo(Identifier id) const final { return nullptr; }
		virtual std::shared_ptr<IResourceBuffer> Open(Identifier id) final { return nullptr; }
	};
}
