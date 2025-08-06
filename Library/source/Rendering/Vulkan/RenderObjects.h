#pragma once
#include "Engine/Array.h"
#include "Engine/SmartPointers.h"

namespace Rendering {
	struct RenderObjectsBase {
		virtual ~RenderObjectsBase() = default;
	};

	using RenderObjectsHandle = ::std::shared_ptr<RenderObjectsBase>;
	using RenderObjectsHandleCollection = ::std::vector<RenderObjectsHandle>;

	/** Collect RenderObjectsHandle types together into a single collection */
	template<std::derived_from<RenderObjectsBase> OtherType, typename AllocatorType>
	RenderObjectsHandleCollection& operator<<(RenderObjectsHandleCollection& collection, std::vector<std::shared_ptr<OtherType>, AllocatorType>&& other) {
		collection.append_range(other);
		return collection;
	}
}
