#pragma once
#include "Engine/Array.h"
#include "Engine/Optional.h"
#include "Rendering/Vulkan/QueueReference.h"

namespace Rendering {
	/**
	 * Requests to create a number of queues from various families.
	 * Queue references can be added arbitrarily, they will be coordinated internally to determine the unique queues being requested.
	 */
	struct QueueRequests {
		/** A request to create some number of queues within a particular family. */
		struct Request {
			/** The family from which to request queues */
			uint32_t id = 0;
			/** The number of queues from the family that are being requested */
			uint32_t count = 0;
		};

		QueueRequests& operator+=(QueueReference reference);

		auto cbegin() const { return requests.cbegin(); }
		auto cend() const { return requests.cend(); }
		auto size() const { return requests.size(); }
		const auto& operator[](size_t index) const { return requests[index]; }

	private:
		std::vector<Request> requests;
	};
}
