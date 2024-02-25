#pragma once
#include "Engine/StandardTypes.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct DescriptorPool {
	public:
		DescriptorPool(VkDevice device, std::span<VkDescriptorPoolSize const> sizes, uint32_t maxNumSets);
		DescriptorPool(DescriptorPool const&) = delete;
		DescriptorPool(DescriptorPool&&) noexcept = default;
		~DescriptorPool();

		inline operator VkDescriptorPool() const { return pool; }

	private:
		stdext::move_only<VkDevice> device;
		VkDescriptorPool pool = nullptr;
	};

	/** A collection of descriptor sets allocated from a descriptor pool */
	template<size_t Size>
	struct DescriptorSets {
	public:
		DescriptorSets(VkDevice device, VkDescriptorPool pool, std::array<VkDescriptorSetLayout, Size> layouts) {
			VkDescriptorSetAllocateInfo descriptorSetAI{};
			descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAI.descriptorPool = pool;
			descriptorSetAI.descriptorSetCount = Size;
			descriptorSetAI.pSetLayouts = layouts.data();

			//Descriptor pools are set up with the assumption that sets will not be individually freed.
			//With that in mind, this class just needs to allocate the sets and nothing else.
			if (vkAllocateDescriptorSets(device, &descriptorSetAI, sets.data()) != VK_SUCCESS || !sets[0]) {
				throw std::runtime_error{ "Could not allocate descriptor sets" };
			}
		}
		DescriptorSets(DescriptorSets const&) = delete;
		DescriptorSets(DescriptorSets&& other) = default;

		inline VkDescriptorSet operator[](size_t index) const { return sets[index]; }

	private:
		std::array<VkDescriptorSet, Size> sets;
	};
}
