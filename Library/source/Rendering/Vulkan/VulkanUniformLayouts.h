#pragma once
#include "Engine/Context.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"

namespace Rendering {
	struct VulkanUniformLayouts {
		/** Uniform layouts */
		VkDescriptorSetLayout global;
		VkDescriptorSetLayout object;

		bool Create(CTX_ARG, VulkanLogicalDevice const& logical);
		void Destroy(VulkanLogicalDevice const& logical);
	};
}
