#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/Flags.h"
#include "Engine/Optional.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct PhysicalDeviceDescription;

	/** Flags that describe how a queue can be used */
	enum struct EQueueFlags : uint8_t {
		Present,
		Graphics,
		Transfer,
		SparseBinding,
		Compute,
	};
	/** Flags that describe how a queue can be used */
	DEFINE_FLAGS_STRUCT(QueueFlags) {
		using TFlags::TFlags;

		/** Create a set of flags based on the raw vulkan flags information */
		static FQueueFlags Create(VkQueueFlags flags);
	};

	/** Information about a queue family on a device */
	struct QueueFamilyDescription {
		/** The unique ID of this family */
		uint32_t id = 0;
		/** The number of queues in this family */
		uint32_t size = 0;
		/** The flags that describe how the queues in this family can be used */
		FQueueFlags flags;
	};

	/** Reference to a specific queue on a device */
	struct QueueReference {
		/** The unique ID of the family to which this queue belongs */
		uint32_t id = 0;
		/** The index of this queue within its family */
		uint32_t index = 0;

		inline bool operator==(QueueReference const&) const = default;
	};

	/** Non-owning handle to a specific queue that was created on a device and which can be used for present operations */
	struct PresentQueue : public QueueReference {
		PresentQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Present on this queue */
		void Present(VkPresentInfoKHR const& info) const;

	protected:
		VkQueue queue = nullptr;
	};

	/** Non-owning handle to a specific queue that was created on a device and which can be used for general graphics operations */
	struct GraphicsQueue : public QueueReference {
		GraphicsQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Submit commands on this queue */
		void Submit(VkSubmitInfo const& info, VkFence fence) const;

	protected:
		VkQueue queue = nullptr;
	};

	/** Non-owning handle to a specific queue that was created on a device and which can be used for transfer operations */
	struct TransferQueue : public QueueReference {
		TransferQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Submit commands on this queue */
		void Submit(VkSubmitInfo const& info, VkFence fence) const;

	protected:
		VkQueue queue = nullptr;
	};

	/** Non-owning handle to a specific queue that was created on a device and which can be used for compute operations */
	struct ComputeQueue : public QueueReference {
		ComputeQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Submit commands on this queue */
		void Submit(VkSubmitInfo const& info, VkFence fence) const;

	protected:
		VkQueue queue = nullptr;
	};

	/** A collection of queues that can be used for surface-related operations on a device */
	struct SurfaceQueues {
		/** The queue that can be used for present operations */
		PresentQueue present;
		/** The queue that can be used for general graphics operations */
		GraphicsQueue graphics;

		/** A collection of references to usable surface queues. Can be used to create or find those qeues. */
		struct References {
			/** A reference to the queue that can be used for present operations */
			QueueReference present;
			/** A reference to the queue that can be used for general graphics operations */
			QueueReference graphics;
		};
	};

	/** A collection of queues that can be used for general shared purposes on a device */
	struct SharedQueues {
		/** The queues that can be used for transfer operations */
		std::vector<TransferQueue> transfers;
		/** The queues that can be used for compute operations */
		std::vector<ComputeQueue> computes;

		struct References {
			/** References to the queues that can be used for transfer operations */
			std::vector<QueueReference> transfers;
			/** References to the queues that can be used for compute operations */
			std::vector<QueueReference> computes;
		};
	};

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
