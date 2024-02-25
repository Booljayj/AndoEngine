#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/Queues.h"

namespace Rendering {
	/** A description of a physical device */
	struct PhysicalDeviceDescription {
		/** The properties of this physical device (such as device limits and API versions) */
		VkPhysicalDeviceProperties properties;
		/** The features available on this physical device */
		VkPhysicalDeviceFeatures features;
		/** The extensions supported by this physical device */
		std::vector<VkExtensionProperties> extensions;

		/** Queue families on this device that can be used for graphics operations */
		std::vector<QueueFamilyDescription> families;

		PhysicalDeviceDescription(VkPhysicalDevice device);
		inline operator VkPhysicalDevice() const { return device; }
		inline bool operator==(VkPhysicalDevice other) const { return device == other; }

		bool SupportsExtension(char const* extension) const;

		/** Get the descriptions of queue families including surface-specific properties */
		t_vector<QueueFamilyDescription> GetSurfaceFamilies(VkSurfaceKHR surface) const;

	private:
		VkPhysicalDevice device = nullptr;
	};

	/** Details related to presenting on a physical device */
	struct PhysicalDevicePresentation {
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;

		static std::optional<PhysicalDevicePresentation> GetPresentation(PhysicalDeviceDescription const& physical, VkSurfaceKHR surface);
	};

	/** Details related to how a physical device can render to a surface */
	struct PhysicalDeviceCapabilities {
		PhysicalDeviceCapabilities(VkPhysicalDevice physical, VkSurfaceKHR surface);
	
		/** Get the minimum number of swapchain images that can be created for this device */
		uint32_t GetImageCountMinimum() const;
		/** Get the swap extent based on the desired size and the current device extent */
		glm::u32vec2 GetSwapExtent(VkSurfaceKHR const& surface, glm::u32vec2 const& desiredExtent) const;
		/** Get the pre transform to use on this device */
		VkSurfaceTransformFlagBitsKHR GetPreTransform(VkSurfaceKHR const& surface) const;
		
	private:
		VkSurfaceCapabilitiesKHR capabilities;
	};
}
