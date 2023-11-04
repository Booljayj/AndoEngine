#pragma once
#include "Engine/Flags.h"
#include "Engine/StandardTypes.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct PhysicalDeviceDescription;

	enum struct EQueueFlags : uint8_t {
		Present,
		Graphics,
		Transfer,
		Compute,
	};
	struct FQueueFlags : public TFlags<EQueueFlags> {
		TFLAGS_METHODS(FQueueFlags);

		static FQueueFlags Parse(VkQueueFlags flags);
	};

	/** Information about a queue family on a device */
	struct QueueFamilyDescription {
		FQueueFlags flags = FQueueFlags::None();
		uint32_t count = 0;
	};

	/** Reference to a specific queue on a device */
	struct QueueReference {
		uint32_t family = std::numeric_limits<uint32_t>::max();
		uint32_t index = 0;

		inline bool operator==(QueueReference const& other) const { return family == other.family && index == other.index; }
	};

	/** A specific queue that was created on a device */
	struct Queue : public QueueReference {
		Queue(VkQueue inQueue, QueueReference inReference) : QueueReference(inReference), queue(inQueue) {}

		inline operator VkQueue() const { return queue; }

	private:
		VkQueue queue = nullptr;
	};

	/** Requests used to create a number of queues for various families */
	struct QueueRequests {
		struct Request {
			uint32_t family = 0;
			uint32_t count = 0;
		};

		QueueRequests& operator+=(QueueReference reference);
		Request const& operator[](size_t index) const { return requests[index]; }

		size_t Size() const { return requests.size(); }

	private:
		std::vector<Request> requests;
	};

	/** A collection of resulting queues that were created for several families */
	struct QueueResults {
		QueueResults() = default;
		QueueResults(VkDevice device, QueueRequests const& requests);

		/** Resize the results to hold the specified number of families */
		void Resize(size_t size) { results.resize(size); }
		/** Attempt to find a queue that is part of these results */
		std::optional<Queue> Find(QueueReference reference) const;
		
	private:
		struct Result {
			uint32_t family = 0;
			std::vector<VkQueue> queues;
		};
		std::vector<Result> results;
	};

	/** A collection of queues that can be used for surface-specific operations on a device */
	struct SurfaceQueues {
		Queue present;
		Queue graphics;

		struct References {
			QueueReference present;
			QueueReference graphics;

			/** Find references to queues that can be used from the set of families. */
			static std::optional<References> Find(std::span<QueueFamilyDescription const> families);

			friend QueueRequests& operator<<(QueueRequests& requests, References const& references);

			/** Resolve these references into the actual queues by searching for them in the results */
			std::optional<SurfaceQueues> ResolveFrom(QueueResults const& results) const;
		};
	};

	/** A collection of queues that can be used for general shared purposes on a device */
	struct SharedQueues {
		Queue transfer;
		std::vector<Queue> computes;

		struct References {
			QueueReference transfer;
			std::vector<QueueReference> computes;

			/** Find references to queues that can be used from the set of families. Queues already used for surface rendering will be avoided if possible. */
			static std::optional<References> Find(std::span<QueueFamilyDescription const> families, SurfaceQueues::References const& surface);
			/** Find references to queues that can be used from the set of families. Used in headless mode where surface rendering is not needed. */
			static std::optional<References> FindHeadless(std::span<QueueFamilyDescription const> families);

			friend QueueRequests& operator<<(QueueRequests& requests, References const& references);

			/** Resolve these references into the actual queues by searching for them in the results */
			std::optional<SharedQueues> ResolveFrom(QueueResults const& results) const;
		};
	};
}
