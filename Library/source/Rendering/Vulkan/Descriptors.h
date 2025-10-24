#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct DescriptorPool {
	public:
		DescriptorPool(VkDevice device, std::span<VkDescriptorPoolSize const> sizes, uint32_t max_num_sets);
		DescriptorPool(DescriptorPool const&) = delete;
		DescriptorPool(DescriptorPool&&) noexcept = default;
		~DescriptorPool();

		inline operator VkDescriptorPool() const { return pool; }

	private:
		MoveOnly<VkDevice> device;
		VkDescriptorPool pool = nullptr;
	};

	/** A collection of descriptor sets allocated from a descriptor pool */
	template<size_t Size>
	struct DescriptorSets {
	public:
		DescriptorSets(VkDevice device, VkDescriptorPool descriptor_pool, std::array<VkDescriptorSetLayout, Size> layouts) {
			VkDescriptorSetAllocateInfo const info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = descriptor_pool,
				.descriptorSetCount = layouts.size(),
				.pSetLayouts = layouts.data(),
			};

			//Descriptor pools are set up with the assumption that sets will not be individually freed.
			//With that in mind, this class just needs to allocate the sets and nothing else.
			if (vkAllocateDescriptorSets(device, &info, sets.data()) != VK_SUCCESS || !sets[0]) {
				throw std::runtime_error{ "Could not allocate descriptor sets" };
			}
		}
		DescriptorSets(DescriptorSets const&) = delete;
		DescriptorSets(DescriptorSets&&) = default;

		inline VkDescriptorSet operator[](size_t index) const { return sets[index]; }

	private:
		std::array<VkDescriptorSet, Size> sets;
	};
}
