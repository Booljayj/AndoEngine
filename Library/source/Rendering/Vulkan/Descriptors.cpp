#include "Rendering/Vulkan/Descriptors.h"

namespace Rendering {
	DescriptorPool::DescriptorPool(VkDevice device, std::span<VkDescriptorPoolSize const> sizes, uint32_t max_num_sets)
		: device(device)
	{
		VkDescriptorPoolCreateInfo const info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.maxSets = max_num_sets,
			.poolSizeCount = static_cast<uint32_t>(sizes.size()),
			.pPoolSizes = sizes.data(),
		};
		
		if (vkCreateDescriptorPool(device, &info, nullptr, &pool) != VK_SUCCESS || !pool) {
			throw std::runtime_error{ "Failed to create descriptor pool for resources" };
		}
	}

	DescriptorPool::~DescriptorPool() {
		if (device) vkDestroyDescriptorPool(device, pool, nullptr);
	}
}
