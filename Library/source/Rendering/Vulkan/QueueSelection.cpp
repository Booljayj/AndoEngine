#include "Rendering/Vulkan/QueueSelection.h"

namespace Rendering {
	QueueReference QueueFamilySelector::Next() {
		if (used >= size) throw std::range_error{ "out of bounds access from queue selector" };
		const QueueReference reference{ id, used };
		++used;
		return reference;
	}
	
	QueueFamilySelectors::QueueFamilySelectors(std::span<QueueFamilyDescription const> descriptions)
		: selectors(ranges::to<std::vector<QueueFamilySelector>>(descriptions))
	{}

	void QueueFamilySelectors::Reset() {
		for (auto& selector : selectors) selector.used = 0;
	}

	std::optional<SurfaceQueues::References> QueueFamilySelectors::SelectSurfaceQueues() {
		//Prefer a single queue that can be used for present and graphics operations
		{
			auto const IsGraphicsAndPresent = [](FQueueFlags flags) { return flags.HasAll(EQueueFlags::Present, EQueueFlags::Graphics); };

			if (auto* selector = FindUnused(IsGraphicsAndPresent)) {
				QueueReference const queue = selector->Next();
				return SurfaceQueues::References{ queue, queue };
			}
		}

		//Fall back to using separate queues for present and graphics operations. We must have one of each.
		{
			auto const IsPresent = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Present); };
			auto const IsGraphics = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Graphics); };

			if (auto* present_selector = FindUnused(IsPresent)) {
				QueueReference const present = present_selector->Next();

				if (auto* graphics_selector = FindUnused(IsGraphics)) {
					QueueReference const graphics = graphics_selector->Next();

					return SurfaceQueues::References{ present, graphics };
				}
			}
		}

		return std::nullopt;
	}

	std::optional<SharedQueues::References> QueueFamilySelectors::SelectSharedQueues(SurfaceQueues::References const& surface) {
		//At the moment, these are the same, because we just want to avoid any queues that were already marked as used.
		return SelectSharedQueues();
	}

	std::optional<SharedQueues::References> QueueFamilySelectors::SelectSharedQueues() {
		SharedQueues::References results;

		//Get each set of queues.
		//It's highly unlikely that we won't be able to find an appropriate set of queues, so we'll just gather everything and check at the end if they're valid. 
		results.transfers = SelectTransferQueues();
		results.computes = SelectComputeQueues();

		if (results.transfers.size() > 0 && results.computes.size() > 0) return results;
		else return std::nullopt;
	}

	std::vector<QueueReference> QueueFamilySelectors::SelectTransferQueues() {
		constexpr uint32_t MaxTransferQueues = 4;

		std::vector<QueueReference> results;

		auto const IsDedicatedTransfer = [](FQueueFlags flags) { return flags == FQueueFlags{ EQueueFlags::Transfer }; };
		if (auto* selector = FindUnused(IsDedicatedTransfer)) {
			const uint32_t available = (selector->size - selector->used);
			const uint32_t desired = std::min(available, MaxTransferQueues);

			const uint32_t first = selector->used;
			const uint32_t last = selector->used + desired;
			for (uint32_t index = first; index < last; ++index) {
				results.emplace_back(selector->id, index);
			}

			//as long as we have at least one result, we can stop. Even one dedicated transfer queue is better than using non-dedicated queues
			return results;
		}

		auto const IsTransfer = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Transfer); };
		if (auto* selector = FindUnused(IsTransfer)) {
			const uint32_t available = (selector->size - selector->used);
			const uint32_t desired = std::min(available, MaxTransferQueues);

			const uint32_t first = selector->used;
			const uint32_t last = selector->used + desired;
			for (uint32_t index = first; index < last; ++index) {
				results.emplace_back(selector->id, index);
			}

			//as long as we have at least one result, we can stop. Even one dedicated transfer queue is better than using non-dedicated queues
			return results;
		}
		
		return results;
	}

	std::vector<QueueReference> QueueFamilySelectors::SelectComputeQueues() {
		constexpr uint32_t NumComputeQueues = 8;

		std::vector<QueueReference> results;
		uint32_t remaining = NumComputeQueues;

		const auto IsDedicatedCompute = [](FQueueFlags flags) { return flags == FQueueFlags{ EQueueFlags::Compute }; };
		if (auto* selector = FindUnused(IsDedicatedCompute)) {
			const uint32_t available = (selector->size - selector->used);
			const uint32_t desired = std::min(available, remaining);

			const uint32_t first = selector->used;
			const uint32_t last = selector->used + desired;
			for (uint32_t index = first; index < last; ++index) {
				results.emplace_back(selector->id, index);
			}

			remaining -= desired;
		}

		if (remaining > 0) {
			const auto IsCompute = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Compute); };
			if (auto* selector = FindUnused(IsCompute)) {
				const uint32_t available = (selector->size - selector->used);
				const uint32_t desired = std::min(available, remaining);

				const uint32_t first = selector->used;
				const uint32_t last = selector->used + desired;
				for (uint32_t index = first; index < last; ++index) {
					results.emplace_back(selector->id, index);
				}
			}
		}

		return results;
	}
}
