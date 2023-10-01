#include "Rendering/Vulkan/Queues.h"
#include "Rendering/Vulkan/PhysicalDevice.h"

namespace Rendering {
	//Finds the next free index if the references to used queues are part of the same family. Used with pack expansion.
	struct FreeIndexFinder {
		uint32_t family = 0;
		uint32_t index = 0;

		FreeIndexFinder(uint32_t family) : family(family) {}

		FreeIndexFinder& operator+=(QueueReference reference) {
			if (reference.family == family) index = std::max(index, reference.index + 1);
			return *this;
		}
	};

	FQueueFlags FQueueFlags::Parse(VkQueueFlags flags) {
		FQueueFlags result;
		if ((flags & VK_QUEUE_GRAPHICS_BIT) > 0) result += EQueueFlags::Graphics;
		if ((flags & VK_QUEUE_TRANSFER_BIT) > 0) result += EQueueFlags::Transfer;
		if ((flags & VK_QUEUE_COMPUTE_BIT) > 0) result += EQueueFlags::Compute;
		return result;
	}

	QueueRequests& QueueRequests::operator+=(QueueReference reference) {
		auto const iter = std::find_if(requests.begin(), requests.end(), [&](auto const& request) { return request.family == reference.family; });
		if (iter == requests.end()) requests.emplace_back( reference.family, reference.index + 1 );
		else iter->count = std::max(iter->count, reference.index + 1);
		return *this;
	}

	QueueResults::QueueResults(VkDevice device, QueueRequests const& requests) {
		results.clear();
		results.resize(requests.Size());

		for (uint32_t req = 0; req < requests.Size(); ++req) {
			auto const& request = requests[req];
			auto& result = results[req];

			result.family = request.family;
			result.queues.resize(request.count);
			for (uint32_t index = 0; index < request.count; ++index) {
				vkGetDeviceQueue(device, result.family, index, &result.queues[index]);
				if (!result.queues[index]) throw std::runtime_error{ "Unable to get queue for request" };
			}
		}
	}

	std::optional<Queue> QueueResults::Find(QueueReference reference) const {
		auto const iter = std::find_if(results.begin(), results.end(), [&](auto const& result) { return result.family == reference.family; });
		if (iter == results.end() || iter->queues.size() <= reference.index) return std::nullopt;
		else return Queue{ iter->queues[reference.index], reference };
	}

	std::optional<SurfaceQueues::References> SurfaceQueues::References::Find(TArrayView<QueueFamilyDescription> families) {
		struct {
			std::optional<QueueReference> graphics;
			std::optional<QueueReference> present;
		} found;

		auto const FindQueueReference = [&families](auto predicate) -> std::optional<QueueReference> {
			for (uint32_t family = 0; family < families.size(); ++family) {
				QueueFamilyDescription const& description = families[family];
				if (predicate(description.flags)) {
					return QueueReference{ family, 0 };
				}
			}
			return std::nullopt;
		};

		auto const IsGraphicsAndPresent = [](FQueueFlags flags) { return flags.HasAll(EQueueFlags::Present, EQueueFlags::Graphics); };
		auto const IsPresent = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Present); };
		auto const IsGraphics = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Graphics); };

		//Prefer a single queue that can be used for present and graphics operations
		found.present = found.graphics = FindQueueReference(IsGraphicsAndPresent);
		if (!found.present) {
			//Fall back to using separate queues for present and graphics operations. We must have one of each.
			found.present = FindQueueReference(IsPresent);
			found.graphics = FindQueueReference(IsGraphics);

			if (!found.present || !found.graphics) return std::nullopt;
		}

		SurfaceQueues::References result;
		result.present = *found.present;
		result.graphics = *found.graphics;
		return result;
	}

	std::optional<SurfaceQueues> SurfaceQueues::References::ResolveFrom(QueueResults const& results) const {
		struct {
			std::optional<Queue> present;
			std::optional<Queue> graphics;
		} resolved;
		
		resolved.present = results.Find(present);
		resolved.graphics = results.Find(graphics);

		if (resolved.present && resolved.graphics) return SurfaceQueues{ *resolved.present, *resolved.graphics };
		else return std::nullopt;
	}

	std::optional<SharedQueues::References> SharedQueues::References::Find(TArrayView<QueueFamilyDescription> families, SurfaceQueues::References const& surface) {
		struct {
			std::optional<QueueReference> transfer;
			std::vector<QueueReference> computes;
		} found;

		//Step 1: find the transfer family. Prefer a queue that is dedicated to transfer, and fall back to a shared queue
		{
			//Finds an unused queue from a family that matches the predicate
			auto const FindTransferQueueReference = [&families](auto predicate, auto... used) -> std::optional<QueueReference> {
				for (uint32_t family = 0; family < families.size(); ++family) {
					QueueFamilyDescription const& description = families[family];
					if (predicate(description.flags)) {
						//Expand input parameters into the finder
						FreeIndexFinder finder{ family };
						(finder += ... += used);

						if (finder.index < description.count) return QueueReference{ family, finder.index };
						else return std::nullopt;
					}
				}
				return std::nullopt;
			};

			auto const IsDedicatedTransfer = [](FQueueFlags flags) { return flags == FQueueFlags{ EQueueFlags::Transfer }; };
			auto const IsTransfer = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Transfer); };

			found.transfer = FindTransferQueueReference(IsDedicatedTransfer, surface.present, surface.graphics);
			if (!found.transfer) {
				found.transfer = FindTransferQueueReference(IsTransfer, surface.present, surface.graphics);
				if (!found.transfer) {
					//If we can't find a dedicated queue but the graphics queue can be used for transfers, then share that queue
					if (IsTransfer(families[surface.graphics.family].flags)) found.transfer = surface.graphics;
					//Otherwise no suitable transfer queue was found
					else return std::nullopt;
				}
			}
		}

		//Step 2: find the compute queues. Prefer up to 8 unused queues, but if that's not possible allow a single shared queue.
		//This is the last step, so we don't need to remove options as they are selected
		{
			constexpr size_t NumComputeQueues = 8;

			//Finds up to "num" unused queues from a family that matches the predicate
			auto const FindComputeQueueReferences = [&families](auto predicate, std::vector<QueueReference>& output, auto... used) {
				if (output.size() >= NumComputeQueues) return;

				for (uint32_t family = 0; family < families.size(); ++family) {
					QueueFamilyDescription const& description = families[family];
					if (predicate(description.flags)) {
						//Expand input parameters into the the finder
						FreeIndexFinder finder{ family };
						(finder += ... += used);

						//Expand previous selections into the finder
						for (QueueReference const& previous : output) finder += previous;

						//If there are remaining unused queues in this family, add as many as we can until the desired amount is reached
						for (uint32_t index = finder.index; index < description.count; ++index) {
							output.push_back(QueueReference{ family, index });

							if (output.size() >= NumComputeQueues) return;
						}
					}
				}
			};

			const auto IsDedicatedCompute = [](FQueueFlags flags) { return flags == FQueueFlags{ EQueueFlags::Compute }; };
			const auto IsCompute = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Compute); };

			//Find as many dedicated compute queues as we can, up to the limit
			FindComputeQueueReferences(IsDedicatedCompute, found.computes, surface.present, surface.graphics, *found.transfer);
			//Find as many non-dedicated compute queues as we can, up to the limit
			FindComputeQueueReferences(IsCompute, found.computes, surface.present, surface.graphics, *found.transfer);

			if (found.computes.empty()) {
				//If there are no dedicated compute queues that we can find but the graphics queue supports compute, then share the graphics queue
				if (IsCompute(families[surface.graphics.family].flags)) found.computes.push_back(surface.graphics);
				//Otherwise no suitable compute queues were found
				else return std::nullopt;
			}
		}

		SharedQueues::References result;
		result.transfer = *found.transfer;
		result.computes = found.computes;
		return result;
	}

	std::optional<SharedQueues::References> SharedQueues::References::FindHeadless(TArrayView<QueueFamilyDescription> families) {
		struct {
			std::optional<QueueReference> transfer;
			std::vector<QueueReference> computes;
		} found;

		//Step 1: find the transfer family. Prefer a queue that is dedicated to transfer, and fall back to a shared queue
		{
			//Finds an unused queue from a family that matches the predicate
			auto const FindTransferQueueReference = [&families](auto predicate) -> std::optional<QueueReference> {
				for (uint32_t family = 0; family < families.size(); ++family) {
					if (predicate(families[family].flags)) return QueueReference{ family, 0 };
				}
				return std::nullopt;
			};

			auto const IsDedicatedTransfer = [](FQueueFlags flags) { return flags == FQueueFlags{ EQueueFlags::Transfer }; };
			auto const IsTransfer = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Transfer); };

			found.transfer = FindTransferQueueReference(IsDedicatedTransfer);
			if (!found.transfer) {
				found.transfer = FindTransferQueueReference(IsTransfer);
				if (!found.transfer) return std::nullopt;
			}
		}

		//Step 2: find the compute queues. Prefer up to 8 unused queues, but if that's not possible allow a single shared queue.
		//This is the last step, so we don't need to remove options as they are selected
		{
			constexpr size_t NumComputeQueues = 8;

			//Finds up to "num" unused queues from a family that matches the predicate
			auto const FindComputeQueueReferences = [&families](auto predicate, std::vector<QueueReference>& output, auto... used) {
				if (output.size() >= NumComputeQueues) return;

				for (uint32_t family = 0; family < families.size(); ++family) {
					QueueFamilyDescription const& description = families[family];
					if (predicate(description.flags)) {
						//Expand input parameters into the the finder
						FreeIndexFinder finder{ family };
						(finder += ... += used);

						//Expand previous selections into the finder
						for (QueueReference const& previous : output) finder += previous;

						//If there are remaining unused queues in this family, add as many as we can until the desired amount is reached
						for (uint32_t index = finder.index; index < description.count; ++index) {
							output.push_back(QueueReference{ family, index });

							if (output.size() >= NumComputeQueues) return;
						}
					}
				}
			};

			const auto IsDedicatedCompute = [](FQueueFlags flags) { return flags == FQueueFlags{ EQueueFlags::Compute }; };
			const auto IsCompute = [](FQueueFlags flags) { return flags.Has(EQueueFlags::Compute); };

			//Find as many dedicated compute queues as we can, up to the limit
			FindComputeQueueReferences(IsDedicatedCompute, found.computes, *found.transfer);
			//Find as many non-dedicated compute queues as we can, up to the limit
			FindComputeQueueReferences(IsCompute, found.computes, *found.transfer);

			if (found.computes.empty()) return std::nullopt;
		}

		return SharedQueues::References{ *found.transfer, std::move(found.computes) };
	}

	std::optional<SharedQueues> SharedQueues::References::ResolveFrom(QueueResults const& results) const {
		struct {
			std::optional<Queue> transfer;
			std::vector<Queue> computes;
		} resolved;

		resolved.transfer = results.Find(transfer);
		if (!resolved.transfer) return std::nullopt;

		for (size_t index = 0; index < computes.size(); ++index) {
			if (auto const compute = results.Find(computes[index])) resolved.computes.emplace_back(*compute);
			else return std::nullopt;
		}

		return SharedQueues{ *resolved.transfer, std::move(resolved.computes) };
	}

	QueueRequests& operator+=(QueueRequests& requests, SurfaceQueues::References const& references) {
		requests += references.present;
		requests += references.graphics;
		return requests;
	}

	QueueRequests& operator+=(QueueRequests& requests, SharedQueues::References const& references) {
		requests += references.transfer;
		for (auto const& compute : references.computes) requests += compute;
		return requests;
	}
}
