#pragma once
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/Framework.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/QueueRequests.h"
#include "Rendering/Vulkan/SharedQueues.h"
#include "Rendering/Vulkan/SurfaceQueues.h"

namespace Rendering {
	/**
	 * Contains the components of a Vulkan logical device, which is used to communicate with a physical device.
	 * A new logical device is created for each physical device that will be used.
	 */
	struct Device {
		using Features = PhysicalDeviceFeatures const&;
		using NameSpan = std::span<char const* const>;
		
		/** Queues created on this device */
		struct {
			/** The shared queues on this device. Used for computations and system operations. */
			SharedQueues shared;
			/** The surface queues on this device. Used for graphics and presentation operations. */
			std::optional<SurfaceQueues> surface;
		} queues;

		/** Get the provided set of shared queues from this device. */
		std::optional<SharedQueues> GetSharedQueues(SharedQueues::References const& references) const;
		/** Get the provided set of surface queues from this device. */
		std::optional<SurfaceQueues> GetSurfaceQueues(SurfaceQueues::References const& references) const;

		Device(Framework const& framework, PhysicalDeviceDescription const& physical, Features enabled_features, NameSpan enabled_extension_names, SharedQueues::References const& shared_references);
		Device(Framework const& framework, PhysicalDeviceDescription const& physical, Features enabled_features, NameSpan enabled_extension_names, SharedQueues::References const& shared_references, SurfaceQueues::References const& surface_references);
		Device(Device const&) = delete;
		Device(Device&&) noexcept = default;
		~Device();

		inline operator VkDevice() const { return device; }
		inline operator VmaAllocator() const { return allocator; }

#ifdef VULKAN_DEBUG
#define SET_DEBUG_NAME_IMPL(Class, type) inline VkResult SetDebugName(Class object, char const* name) const { return SetDebugName(object, type, name); }
		SET_DEBUG_NAME_IMPL(VkQueue, VK_OBJECT_TYPE_QUEUE);
		SET_DEBUG_NAME_IMPL(VkImage, VK_OBJECT_TYPE_IMAGE);
		SET_DEBUG_NAME_IMPL(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW);
#undef SET_DEBUG_NAME_IMPL
#endif

	private:
		MoveOnly<VkDevice> device;

		VmaAllocator allocator = nullptr;
#ifdef VULKAN_DEBUG
		PFN_vkSetDebugUtilsObjectNameEXT functionSetDebugName = nullptr;
#endif

		Device(Framework const& framework, PhysicalDeviceDescription const& physical, Features enabled_features, NameSpan enabled_extension_names, QueueRequests const& requests);

		VkQueue FindQueue(QueueReference reference) const;

		static QueueRequests GenerateQueueRequests(SharedQueues::References const& shared_references);
		static QueueRequests GenerateQueueRequests(SharedQueues::References const& shared_references, SurfaceQueues::References const& surface_references);

#ifdef VULKAN_DEBUG
		VkResult SetDebugName(void* object, VkObjectType type, char const* name) const;
#endif
	};
}
