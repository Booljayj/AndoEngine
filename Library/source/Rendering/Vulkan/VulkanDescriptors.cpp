#include "Rendering/Vulkan/VulkanDescriptors.h"

namespace Rendering {
	DescriptorPool::DescriptorPool(VkDevice inDevice, TArrayView<VkDescriptorPoolSize> sizes, size_t maxNumSets)
		: device(inDevice)
	{
		VkDescriptorPoolCreateInfo descriptorPoolCI{};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(sizes.size());
		descriptorPoolCI.pPoolSizes = sizes.begin();
		descriptorPoolCI.maxSets = maxNumSets;

		if (vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &pool) != VK_SUCCESS || !pool) {
			throw std::runtime_error{ "Failed to create descriptor pool for resources" };
		}
	}

	DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept {
		std::swap(device, other.device);
		std::swap(pool, other.pool);
	}

	DescriptorPool::~DescriptorPool() {
		if (device) vkDestroyDescriptorPool(device, pool, nullptr);
	}
}
