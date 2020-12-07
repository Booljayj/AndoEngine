#pragma once
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanFramework.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"

namespace Rendering {
	/**
	 * Contains the components of a Vulkan logical device, which is used to communicate with a physical device.
	 * A new logical device is created for each physical device that will be used.
	 */
	struct VulkanLogicalDevice {
		/** The underlying logical device */
		VkDevice device = nullptr;

		struct {
			VkQueue present = nullptr;
			VkQueue graphics = nullptr;
		} queues;

		/** The allocator for device memory */
		VmaAllocator allocator;

		VulkanLogicalDevice() = default;
		VulkanLogicalDevice(VulkanLogicalDevice&& other);

		VulkanLogicalDevice& operator=(VulkanLogicalDevice&& other);
		inline operator bool() const { return device && allocator; }

		static VulkanLogicalDevice Create(CTX_ARG, VulkanFramework framework, VulkanPhysicalDevice const& physical, VkPhysicalDeviceFeatures const& features, TArrayView<char const*> const& extensions);
		void Destroy();

	private:
		static VmaAllocatorCreateFlags GetAllocatorFlags();
	};
}
