#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Engine/Utility.h"

namespace Rendering {
	VulkanPhysicalDevice VulkanPhysicalDevice::Get(CTX_ARG, VkPhysicalDevice const& device, VkSurfaceKHR const& surface) {
		TEMP_ALLOCATOR_MARK();

		VulkanPhysicalDevice Result;
		Result.device = device;
		vkGetPhysicalDeviceProperties(device, &Result.properties);
		vkGetPhysicalDeviceFeatures(device, &Result.features);

		{
			uint32_t extensionCount = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			VkExtensionProperties* extensions = CTX.temp.Request<VkExtensionProperties>(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions);

			Result.supportedExtensions.reserve(extensionCount);
			for (uint32_t index = 0; index < extensionCount; ++index) {
				Result.supportedExtensions.push_back(std::string{extensions[index].extensionName});
			}
		}
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &Result.presentation.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
			Result.presentation.surfaceFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, Result.presentation.surfaceFormats.data());

			uint32_t modeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);
			Result.presentation.presentModes.resize(modeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, Result.presentation.presentModes.data());
		}
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
			VkQueueFamilyProperties* queueFamilies = CTX.temp.Request<VkQueueFamilyProperties>(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

			for (uint32_t queueIndex = 0; queueIndex < queueFamilyCount; ++queueIndex) {
				const VkQueueFamilyProperties& queueFamily = queueFamilies[queueIndex];

				VkBool32 bIsSupportedForPresentation = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, queueIndex, surface, &bIsSupportedForPresentation);
				if (bIsSupportedForPresentation) {
					Result.queues.present = { queueIndex, queueFamily.queueCount };
				}

				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					Result.queues.graphics = { queueIndex, queueFamily.queueCount };
				}

				//Stop the loop if we have managed to find all the required queues
				if (Result.HasRequiredQueues()) break;
			}
		}

		return Result;
	}

	TArrayView<char const*> VulkanPhysicalDevice::GetExtensionNames(CTX_ARG) {
		return {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		};
	}

	bool VulkanPhysicalDevice::HasRequiredQueues() const {
		return queues.graphics.has_value() && queues.present.has_value();
	}

	bool VulkanPhysicalDevice::HasRequiredExtensions(TArrayView<char const*> const& requiredExtensionNames) const {
		for (char const* requiredExtensionName : requiredExtensionNames) {
			bool foundExtension = false;
			for (std::string const& supportedExtension : supportedExtensions) {
				if (strcmp(requiredExtensionName, supportedExtension.c_str()) == 0) {
					foundExtension = true;
					break;
				}
			}
			if (!foundExtension) return false;
		}
		return true;
	}

	bool VulkanPhysicalDevice::HasSwapchainSupport() const {
		return !presentation.surfaceFormats.empty() && !presentation.presentModes.empty();
	}

	VkExtent2D VulkanPhysicalDevice::GetSwapExtent(VkSurfaceKHR const& surface, VkExtent2D const& desiredExtent) const {
		//Get the current device capabilities, which includes the current size if it has been set
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		} else {
			VkExtent2D actualExtent;
			actualExtent.width = std::clamp(desiredExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(desiredExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}

	VkSurfaceTransformFlagBitsKHR VulkanPhysicalDevice::GetPreTransform(VkSurfaceKHR const& surface) const {
		//Get the current device capabilities, which includes the current preTransform if it has been set
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		} else {
			return capabilities.currentTransform;
		}
	}
}
