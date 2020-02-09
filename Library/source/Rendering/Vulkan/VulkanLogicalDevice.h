#pragma once
#include <vulkan/vulkan.h>
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"

namespace Rendering {
	struct VulkanLogicalDevice {
		/** The underlying logical device */
		VkDevice device = nullptr;
		/** The features that are actually enabled for this device */
		VkPhysicalDeviceFeatures enabledFeatures;

		struct {
			VkQueue present = nullptr;
			VkQueue graphics = nullptr;
		} queues;

		bool Create(CTX_ARG, Rendering::VulkanPhysicalDevice const& physicalDevice, VkPhysicalDeviceFeatures const& inEnabledFeatures);
		void Destroy();
	};
}
