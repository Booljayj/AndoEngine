#pragma once
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"

namespace Rendering {
	struct VulkanUniformLayouts {
		/** Uniform layouts */
		VkDescriptorSetLayout global = nullptr;
		VkDescriptorSetLayout object = nullptr;

		bool Create(VulkanLogicalDevice const& logical);
		void Destroy(VulkanLogicalDevice const& logical);
	};
}
