#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/GLM.h"
#include "Engine/TemporaryContainers.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/QueueFamily.h"

namespace Rendering {
	struct PhysicalDeviceFeatures {
		VkPhysicalDeviceFeatures2 version10 = {};
		VkPhysicalDeviceVulkan11Features version11 = {};
		VkPhysicalDeviceVulkan12Features version12 = {};
		
		PhysicalDeviceFeatures();
		PhysicalDeviceFeatures(VkPhysicalDevice device);
		PhysicalDeviceFeatures(const PhysicalDeviceFeatures& other);
	};

	/** The capabilities of a physical device when rendering to a specific surface. */
	struct PhysicalDeviceCapabilities {
		PhysicalDeviceCapabilities(VkPhysicalDevice physical, VkSurfaceKHR surface);

		/** Get the minimum number of swapchain images that can be created for this device */
		uint32_t GetImageCountMinimum() const;
		/** Get the swap extent based on the desired size and the current device extent */
		glm::u32vec2 GetSwapExtent(glm::u32vec2 const& desiredExtent) const;
		/** Get the pre transform to use on this device */
		VkSurfaceTransformFlagBitsKHR GetPreTransform() const;

	private:
		VkSurfaceCapabilitiesKHR capabilities;
	};

	/** A description of a physical device */
	struct PhysicalDeviceDescription {
		/** The properties of this physical device (such as device limits and API versions) */
		VkPhysicalDeviceProperties properties;

		/** The features available on this physical device */
		PhysicalDeviceFeatures supported_features;
		/** The extensions supported by this physical device */
		std::vector<VkExtensionProperties> supported_extensions;

		/** Queue families on this device that can be used for graphics operations */
		std::vector<QueueFamilyDescription> families;

		PhysicalDeviceDescription(VkPhysicalDevice device);
		inline operator VkPhysicalDevice() const { return device; }
		inline bool operator==(VkPhysicalDevice other) const { return device == other; }

		bool SupportsExtension(char const* extension_name) const;

		/** Get the capabilities of this physical device when rendering to the specified surface. */
		PhysicalDeviceCapabilities GetSurfaceCapabilities(VkSurfaceKHR surface) const;

		/** Get the descriptions of queue families including surface-specific properties */
		std::vector<QueueFamilyDescription> GetSurfaceQueueFamilies(VkSurfaceKHR surface) const;

		/** Get the surface formats that this physical device supports for the given surface */
		std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR surface) const;
		/** Get the presentation modes that this physical device supports for the given surface */
		std::vector<VkPresentModeKHR> GetSurfacePresentModes(VkSurfaceKHR surface) const;

	private:
		VkPhysicalDevice device = nullptr;
	};
}
