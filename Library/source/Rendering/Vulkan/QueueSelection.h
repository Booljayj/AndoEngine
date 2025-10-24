#pragma once
#include "Engine/Core.h"
#include "Engine/Optional.h"
#include "Engine/Tuple.h"
#include "Rendering/Vulkan/QueueFamily.h"
#include "Rendering/Vulkan/QueueReference.h"
#include "Rendering/Vulkan/SharedQueues.h"
#include "Rendering/Vulkan/SurfaceQueues.h"

namespace Rendering {
	/** Helper type used to search for available queues that match certain criteria. */
	struct QueueFamilySelector : public QueueFamilyDescription {
		uint32_t used = 0;

		QueueFamilySelector(const QueueFamilyDescription& description) : QueueFamilyDescription(description) {};

		/** Return the reference to the next unused queue in this family. */
		QueueReference Next();
	};

	struct QueueFamilySelectors {
		QueueFamilySelectors(std::span<QueueFamilyDescription const> descriptions);

		/** Reset all usage markers on the queues. This should be called before trying a different queue selection strategy after selections have already been made. */
		void Reset();

		/** Attempt to select a set of surface queues. These queues will be marked as "used". */
		std::optional<SurfaceQueues::References> SelectSurfaceQueues();

		/** Attempt to select a set of surface queues. These queues will be marked as "used". */
		std::optional<SharedQueues::References> SelectSharedQueues(SurfaceQueues::References const& surface);
		/** Attempt to select a set of surface queues. These queues will be marked as "used". Used in cases where there are no surface queues, such as headless rendering. */
		std::optional<SharedQueues::References> SelectSharedQueues();

	private:
		std::vector<QueueFamilySelector> selectors;

		/** Find the selector for a queue family that matches the predicate, regardless of whether it has unused queues. */
		template<std::predicate<FQueueFlags> PredicateType>
		QueueFamilySelector* Find(PredicateType&& predicate) {
			const auto iter = ranges::find_if(selectors, [&](QueueFamilySelector const& selector) { return predicate(selector.flags); });
			if (iter != selectors.end()) return std::addressof(*iter);
			else return nullptr;
		}

		/** Find the selector for a queue family that matches the predicate, only including selectors that have unused queues. */
		template<std::predicate<FQueueFlags> PredicateType>
		QueueFamilySelector* FindUnused(PredicateType&& predicate) {
			const auto iter = ranges::find_if(selectors, [&](QueueFamilySelector const& selector) { return selector.used < selector.size && predicate(selector.flags); });
			if (iter != selectors.end()) return std::addressof(*iter);
			else return nullptr;
		}

		std::vector<QueueReference> SelectTransferQueues();
		std::vector<QueueReference> SelectComputeQueues();
	};
}