#pragma once
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/Framework.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/QueueRequests.h"

namespace Rendering {
	/**
	 * Contains the components of a Vulkan logical device, which is used to communicate with a physical device.
	 * A new logical device is created for each physical device that will be used.
	 */
	struct Device {
		using ExtensionsView = std::span<char const* const>;
		
		/** Queues created on this device */
		QueueResults queues;

		Device(Framework const& framework, PhysicalDeviceDescription const& physical, VkPhysicalDeviceFeatures features, ExtensionsView extensions, QueueRequests const& requests);
		Device(Device const&) = delete;
		Device(Device&&) noexcept = default;
		~Device();

		inline operator VkDevice() const { return device; }
		inline operator VmaAllocator() const { return allocator; }

		/** Get the physical device backing this logical device */
		PhysicalDeviceDescription const& GetPhysical() const { return physical; }

#ifdef VULKAN_DEBUG
#define SET_DEBUG_NAME_IMPL(Class, type) inline VkResult SetDebugName(Class object, char const* name) const { return SetDebugName(object, type, name); }
		SET_DEBUG_NAME_IMPL(VkQueue, VK_OBJECT_TYPE_QUEUE);
		SET_DEBUG_NAME_IMPL(VkImage, VK_OBJECT_TYPE_IMAGE);
		SET_DEBUG_NAME_IMPL(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW);
#undef SET_DEBUG_NAME_IMPL
#endif

	private:
		PhysicalDeviceDescription const& physical;
		MoveOnly<VkDevice> device;
		VmaAllocator allocator = nullptr;
#ifdef VULKAN_DEBUG
		PFN_vkSetDebugUtilsObjectNameEXT functionSetDebugName = nullptr;
#endif

#ifdef VULKAN_DEBUG
		VkResult SetDebugName(void* object, VkObjectType type, char const* name) const;
#endif
	};
}
