#pragma once
#include "Rendering/Vulkan/Vulkan.h"
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
		VmaAllocator allocator = nullptr;

		VulkanLogicalDevice() = default;
		VulkanLogicalDevice(VulkanLogicalDevice&& other);

		VulkanLogicalDevice& operator=(VulkanLogicalDevice&& other);
		inline operator VkDevice() const { return device; }
		inline operator bool() const { return device && allocator; }

		static VulkanLogicalDevice Create(VulkanFramework framework, VulkanPhysicalDevice const& physical, VkPhysicalDeviceFeatures const& features, TArrayView<char const*> const& extensions);
		void Destroy();

#ifdef VULKAN_DEBUG
#define SET_DEBUG_NAME_IMPL(Class, type) inline VkResult SetDebugName(Class object, char const* name) const { return SetDebugName(object, type, name); }
		SET_DEBUG_NAME_IMPL(VkQueue, VK_OBJECT_TYPE_QUEUE);
#undef SET_DEBUG_NAME_IMPL
#endif

	private:
#ifdef VULKAN_DEBUG
		PFN_vkSetDebugUtilsObjectNameEXT functionSetDebugName = nullptr;
		VkResult SetDebugName(void* object, VkObjectType type, char const* name) const;
#endif

		static VmaAllocatorCreateFlags GetAllocatorFlags();
	};
}
