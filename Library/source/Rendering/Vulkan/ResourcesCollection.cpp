#include "ResourcesCollection.h"
#include "Engine/Ranges.h"
#include "Rendering/Vulkan/ResourcesCollection.h"

namespace Rendering {
	ResourcesCollection& ResourcesCollection::operator<<(ResourcesCollection& other) {
		MoveAppend(graphics_pipelines, other.graphics_pipelines);
		MoveAppend(meshes, other.meshes);
		return *this;
	}

	bool ResourcesCollection::empty() const {
		return
			graphics_pipelines.empty() &&
			meshes.empty();
	}

	void ResourcesCollection::clear() {
		graphics_pipelines.clear();
		meshes.clear();
	}

	void ResourcesCollection::reserve(size_t num) {
		graphics_pipelines.reserve(num);
		meshes.reserve(num);
	}
}
