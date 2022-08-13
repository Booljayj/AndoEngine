#include "Rendering/StaticMeshResource.h"

DEFINE_REFLECT_STRUCT(Rendering, StaticMeshResource);

namespace Rendering {

	bool StaticMeshResourceDatabase::Startup(const RenderingSystem& inRendering) {
		rendering = &inRendering;
		return true;
	}

	bool StaticMeshResourceDatabase::Shutdown() {
		std::unique_lock lock{ mutex };
		for (Resources::Resource* resource : resources) Destroy(*resource);
		ids.clear();
		resources.clear();
		return true;
	}

	void StaticMeshResourceDatabase::CollectGarbage() {
		// std::unique_lock lock{ mutex };

		// for (auto iter = resources.rbegin(); iter != resources.rend(); ++iter) {
		// 	//If the use count is 1, then this shared_ptr is the only reference to the resource.
		// 	//The entries array is locked, so we can't create new references concurrently.
		// 	if (iter-) {
		// 		auto const last = entries.rbegin();
		// 		std::iter_swap(iter, last);
		// 		Destroy(*last->resource);
		// 		entries.pop_back();
		// 	}
		// }
	}

	void StaticMeshResourceDatabase::PostCreate(StaticMeshResource& resource) {

	}

	void StaticMeshResourceDatabase::PostLoad(StaticMeshResource& resource) {

	}

	void StaticMeshResourceDatabase::Destroy(Resources::Resource& resource) {

	}
}
