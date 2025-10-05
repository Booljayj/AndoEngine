#include "Rendering/Vulkan/Queues.h"
#include "Rendering/Vulkan/PhysicalDevice.h"

namespace Rendering {
	//Finds the next free index if the references to used queues are part of the same family. Used with pack expansion.
	struct FreeIndexFinder {
		uint32_t familyID = 0;
		uint32_t index = 0;

		FreeIndexFinder(uint32_t familyID) : familyID(familyID) {}

		FreeIndexFinder& operator+=(QueueReference reference) {
			if (reference.id == familyID) index = std::max(index, reference.index + 1);
			return *this;
		}
	};

	FQueueFlags FQueueFlags::Create(VkQueueFlags flags) {
		FQueueFlags result;
		if ((flags & VK_QUEUE_GRAPHICS_BIT) > 0) result += EQueueFlags::Graphics;
		if ((flags & VK_QUEUE_TRANSFER_BIT) > 0) result += EQueueFlags::Transfer;
		if ((flags & VK_QUEUE_SPARSE_BINDING_BIT) > 0) result += EQueueFlags::SparseBinding;
		if ((flags & VK_QUEUE_COMPUTE_BIT) > 0) result += EQueueFlags::Compute;
		return result;
	}

	void PresentQueue::Present(VkPresentInfoKHR const& info) const {
		if (vkQueuePresentKHR(queue, &info) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to present to queue" };
		}
	}

	void GraphicsQueue::Submit(VkSubmitInfo const& info, VkFence fence) const {
		if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to submit commands to queue" };
		}
	}

	void TransferQueue::Submit(VkSubmitInfo const& info, VkFence fence) const {
		if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to submit commands to queue" };
		}
	}

	void ComputeQueue::Submit(VkSubmitInfo const& info, VkFence fence) const {
		if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to submit commands to queue" };
		}
	}

	QueueRequests& QueueRequests::operator+=(QueueReference reference) {
		auto const iter = ranges::find_if(requests, [&](auto const& request) { return request.id == reference.id; });
		if (iter != requests.end()) {
			iter->count = std::max(iter->count, reference.index + 1);
		} else {
			requests.emplace_back(reference.id, reference.index + 1);
		}
		return *this;
	}

	QueueRequests& QueueRequests::operator<<(SurfaceQueues::References const& references) {
		operator+=(references.present);
		operator+=(references.graphics);
		return *this;
	}

	QueueRequests& QueueRequests::operator<<(SharedQueues::References const& references) {
		for (auto const& transfer : references.transfers) operator+=(transfer);
		for (auto const& compute : references.computes) operator+=(compute);
		return *this;
	}

	QueueResults::QueueResults(VkDevice device, QueueRequests const& requests) {
		results.resize(requests.size());

		for (uint32_t req = 0; req < requests.size(); ++req) {
			auto const& request = requests[req];
			auto& result = results[req];

			result.id = request.id;
			result.queues.resize(request.count);
			for (uint32_t index = 0; index < request.count; ++index) {
				vkGetDeviceQueue(device, result.id, index, &result.queues[index]);
				if (!result.queues[index]) throw std::runtime_error{ "Unable to get queue for request" };
			}
		}
	}

	VkQueue QueueResults::Find(QueueReference reference) const {
		auto const iter = ranges::find_if(results, [&](auto const& result) { return result.id == reference.id; });
		if (iter == results.end() || iter->queues.size() <= reference.index) return nullptr;
		else return iter->queues[reference.index];
	}

	std::optional<SurfaceQueues> QueueResults::Resolve(SurfaceQueues::References const& references) const {
		struct {
			std::optional<PresentQueue> present;
			std::optional<GraphicsQueue> graphics;
		} resolved;
		
		if (VkQueue queue = Find(references.present)) resolved.present = PresentQueue{ queue, references.present };
		else return std::nullopt;

		if (VkQueue queue = Find(references.graphics)) resolved.graphics = GraphicsQueue{ queue, references.graphics };
		else return std::nullopt;

		return SurfaceQueues{ *resolved.present, *resolved.graphics };
	}

	std::optional<SharedQueues> QueueResults::Resolve(SharedQueues::References const& references) const {
		SharedQueues result;

		for (QueueReference transfer : references.transfers) {
			if (VkQueue queue = Find(transfer)) result.transfers.emplace_back(queue, transfer);
			else return std::nullopt;
		}
		for (QueueReference compute : references.computes) {
			if (VkQueue queue = Find(compute)) result.computes.emplace_back(queue, compute);
			else return std::nullopt;
		}

		return result;
	}
}
