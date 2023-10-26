#pragma once
#include "Engine/StandardTypes.h"

namespace Rendering {
	struct RenderObjectsBase {
		virtual ~RenderObjectsBase() = default;
	};

	using RenderObjectsHandle = ::std::shared_ptr<RenderObjectsBase>;
	using RenderObjectsHandleCollection = ::std::vector<RenderObjectsHandle>;

	/** Collect RenderObjectsHandle types together into a single collection */
	template<typename OtherType, typename AllocatorType>
		requires std::is_base_of_v<RenderObjectsBase, OtherType>
	RenderObjectsHandleCollection& operator<<(RenderObjectsHandleCollection& collection, std::vector<std::shared_ptr<OtherType>, AllocatorType>&& other) {
		stdext::append(collection, std::move(other));
		return collection;
	}
}
