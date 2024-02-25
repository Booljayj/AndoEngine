#include "Rendering/Vulkan/Descriptors.h"

namespace Rendering {
	DescriptorPool::DescriptorPool(VkDevice device, std::span<VkDescriptorPoolSize const> sizes, uint32_t maxNumSets)
		: device(device)
	{
		VkDescriptorPoolCreateInfo const descriptorPoolCI{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = maxNumSets,
			.poolSizeCount = static_cast<uint32_t>(sizes.size()),
			.pPoolSizes = sizes.data(),
		};
		
		if (vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &pool) != VK_SUCCESS || !pool) {
			throw std::runtime_error{ "Failed to create descriptor pool for resources" };
		}
	}

	DescriptorPool::~DescriptorPool() {
		if (device) vkDestroyDescriptorPool(device, pool, nullptr);
	}
}
