#pragma once
#include <set>
#include <optional>
#include <ostream>
#include <vulkan/vulkan.h>
#include "Engine/Context.h"

namespace Rendering {
	struct VulkanPhysicalDevice {
		struct QueueFamilyInfo {
			uint32_t index;
			uint32_t count;
		};

		/** The underlying device */
		VkPhysicalDevice device = nullptr;

		/** The properties of this physical device (such as device limits and API versions) */
		VkPhysicalDeviceProperties properties;
		/** The features available on this physical device */
		VkPhysicalDeviceFeatures features;
		/** The names of all extensions supported on this physical device */
		std::vector<std::string> extensions;

		/** Details about surface, format, and present mode support on this device */
		struct {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> surfaceFormats;
			std::vector<VkPresentModeKHR> presentModes;
		} presentation;

		/** The optional information for different types of queues on this device */
		struct {
			std::optional<QueueFamilyInfo> present;
			std::optional<QueueFamilyInfo> graphics;
		} queues;

		/** Get the information for a device that will be used with a surface */
		static VulkanPhysicalDevice Get(CTX_ARG, VkPhysicalDevice const& device, VkSurfaceKHR const& surface);

		/** True if the device has all the queues required to be used */
		bool HasRequiredQueues() const;
		/** True if the device has all the extensions required to be used */
		bool HasRequiredExtensions(std::set<std::string_view> const& requiredExtensions) const;
		/** True if the device is capable of creating SwapChains */
		bool HasSwapchainSupport() const;

		/** Get the swap extent based on the desired size and the current device extent */
		VkExtent2D GetSwapExtent(VkSurfaceKHR const& surface, VkExtent2D const& desiredExtent) const;
		/** Get the pre transform to use on this device */
		VkSurfaceTransformFlagBitsKHR GetPreTransform(VkSurfaceKHR const& surface) const;

		/** Write a description of the device and its properties to the stream */
		void WriteDescription(std::ostream& stream) const;
	};
}
