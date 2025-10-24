#pragma once
#include "Engine/Array.h"
#include "Engine/Optional.h"
#include "Rendering/Vulkan/SharedQueues.h"
#include "Rendering/Vulkan/SurfaceQueues.h"

namespace Rendering {
	/** Requests used to create a number of queues from various families */
	struct QueueRequests {
		struct Request {
			/** The family from which to request queues */
			uint32_t id = 0;
			/** The number of queues from the family that are being requested */
			uint32_t count = 0;
		};

		QueueRequests& operator+=(QueueReference reference);
		QueueRequests& operator<<(SurfaceQueues::References const& references);
		QueueRequests& operator<<(SharedQueues::References const& references);

		Request const& operator[](size_t index) const { return requests[index]; }
		size_t size() const { return requests.size(); }

	private:
		std::vector<Request> requests;
	};

	/** A collection of queues that were created on a device */
	struct QueueResults {
		QueueResults() = default;
		QueueResults(VkDevice device, QueueRequests const& requests);

		/** Attempt to find a queue that is part of these results */
		VkQueue Find(QueueReference reference) const;

		std::optional<SurfaceQueues> Resolve(SurfaceQueues::References const& references) const;
		std::optional<SharedQueues> Resolve(SharedQueues::References const& references) const;

	private:
		struct Result {
			/** The unique id of the family to which the queues belong */
			uint32_t id = 0;
			/** The queues that were created in this family */
			std::vector<VkQueue> queues;
		};
		std::vector<Result> results;
	};
}
