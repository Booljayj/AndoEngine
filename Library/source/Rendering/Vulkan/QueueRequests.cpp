#include "Rendering/Vulkan/QueueRequests.h"

namespace Rendering {
	QueueRequests& QueueRequests::operator+=(QueueReference reference) {
		auto const iter = ranges::find_if(requests, [&](auto const& request) { return request.id == reference.id; });
		if (iter != requests.end()) {
			iter->count = std::max(iter->count, reference.index + 1);
		} else {
			requests.emplace_back(reference.id, reference.index + 1);
		}
		return *this;
	}
}
