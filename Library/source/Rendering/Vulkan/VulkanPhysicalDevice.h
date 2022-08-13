#pragma once
#include "Engine/ArrayView.h"
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Contains information about a physical device that can be used for Vulkan rendering */
	struct VulkanPhysicalDevice {
		struct QueueFamilyInfo {
			uint32_t index = std::numeric_limits<uint32_t>::max();
			uint32_t count = std::numeric_limits<uint32_t>::max();
		};

		/** The underlying device */
		VkPhysicalDevice device = nullptr;

		/** The properties of this physical device (such as device limits and API versions) */
		VkPhysicalDeviceProperties properties = {};
		/** The features available on this physical device */
		VkPhysicalDeviceFeatures features = {};
		/** The names of all extensions supported on this physical device */
		std::vector<std::string> supportedExtensions;

		/** Details about surface, format, and present mode support on this device */
		struct {
			VkSurfaceCapabilitiesKHR capabilities = {};
			std::vector<VkSurfaceFormatKHR> surfaceFormats = {};
			std::vector<VkPresentModeKHR> presentModes = {};
		} presentation;

		/** The optional information for different types of queues on this device */
		struct {
			std::optional<QueueFamilyInfo> present;
			std::optional<QueueFamilyInfo> graphics;
		} queues;

		/** Get the information for a device that will be used with a surface */
		static VulkanPhysicalDevice Get(VkPhysicalDevice const& device, VkSurfaceKHR const& surface);

		static t_vector<char const*> GetExtensionNames();

		VulkanVersion GetVulkanVersion() const { return VulkanVersion{properties.apiVersion}; }
		VulkanVersion GetDriverVersion() const { return VulkanVersion{properties.driverVersion}; }

		/** True if the device has all the queues required to be used */
		bool HasRequiredQueues() const;
		/** True if the device has all the extensions required to be used */
		bool HasRequiredExtensions(TArrayView<char const*> const& extensionNames) const;
		/** True if the device is capable of creating SwapChains */
		bool HasSwapchainSupport() const;

		/** Get the swap extent based on the desired size and the current device extent */
		glm::u32vec2 GetSwapExtent(VkSurfaceKHR const& surface, glm::u32vec2 const& desiredExtent) const;
		/** Get the pre transform to use on this device */
		VkSurfaceTransformFlagBitsKHR GetPreTransform(VkSurfaceKHR const& surface) const;
	};
}
