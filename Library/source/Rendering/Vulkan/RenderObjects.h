#pragma once
#include "Engine/StandardTypes.h"

namespace Rendering {
	struct RenderObjectsBase {
		virtual ~RenderObjectsBase() = default;
	};

	using RenderObjectsHandle = ::std::shared_ptr<RenderObjectsBase>;
	using RenderObjectsHandleCollection = ::std::vector<RenderObjectsHandle>;

	/** Collect RenderObjectsHandle types together into a single collection */
	template<std::derived_from<RenderObjectsBase> OtherType, typename AllocatorType>
	RenderObjectsHandleCollection& operator<<(RenderObjectsHandleCollection& collection, std::vector<std::shared_ptr<OtherType>, AllocatorType>&& other) {
		stdext::append(collection, std::move(other));
		return collection;
	}
}
