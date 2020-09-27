#pragma once
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"

namespace Rendering {
	struct VulkanLogicalDevice {
		/** The underlying logical device */
		VkDevice device = nullptr;

		struct {
			VkQueue present = nullptr;
			VkQueue graphics = nullptr;
		} queues;

		bool Create(CTX_ARG, Rendering::VulkanPhysicalDevice const& physicalDevice, VkPhysicalDeviceFeatures const& enabledFeatures, TArrayView<char const*> const& enabledExtensionNames);
		void Destroy();
	};
}
