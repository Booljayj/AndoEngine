#include "Engine/ScopedTempBlock.h"
#include "Engine/Utility.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanVersion.h"

namespace Rendering {
	VulkanPhysicalDevice VulkanPhysicalDevice::Get(CTX_ARG, VkPhysicalDevice const& device, VkSurfaceKHR const& surface) {
		TEMP_SCOPE;
		VulkanPhysicalDevice Result;
		Result.device = device;
		vkGetPhysicalDeviceProperties(device, &Result.properties);
		vkGetPhysicalDeviceFeatures(device, &Result.features);

		{
			uint32_t extensionCount = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			VkExtensionProperties* supportedExtensions = CTX.Temp.Request<VkExtensionProperties>(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, supportedExtensions);

			Result.extensions.reserve(extensionCount);
			for (uint32_t index = 0; index < extensionCount; ++index) {
				Result.extensions.push_back(std::string{supportedExtensions[index].extensionName});
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
			VkQueueFamilyProperties* queueFamilies = CTX.Temp.Request<VkQueueFamilyProperties>(queueFamilyCount);
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

	bool VulkanPhysicalDevice::HasRequiredQueues() const {
		return queues.graphics.has_value() && queues.present.has_value();
	}

	bool VulkanPhysicalDevice::HasRequiredExtensions(std::set<std::string_view> const& requiredExtensions) const {
		uint32_t requiredExtensionsSatisfied = 0;
		for (std::string const& extension : extensions) {
			requiredExtensionsSatisfied += requiredExtensions.count(extension);
		}
		return requiredExtensionsSatisfied == requiredExtensions.size();
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

	void VulkanPhysicalDevice::WriteDescription(std::ostream& stream) const {
		stream << "Device Name: " << properties.deviceName << std::endl;
		stream << "Device Type: " << properties.deviceType << std::endl; //@todo Mimic the behavior of vk::to_string(vk::PhysicalDeviceType) here for better formatting
		stream << "API Version: " << VulkanVersion{properties.apiVersion} << std::endl;
		stream << "Driver Version: " << VulkanVersion{properties.driverVersion} << std::endl;

		stream << "Discovered Queue Families:" << std::endl;
		const auto WriteQueue = [&](char const* name, std::optional<QueueFamilyInfo> info) {
			stream << "\t" << name << ": ";
			if (info.has_value()) {
				stream << "Index: " << info.value().index << ", Count: " << info.value().count << std::endl;
			} else {
				stream << "Not found" << std::endl;
			}
		};
		WriteQueue("graphics", queues.graphics);
		WriteQueue("present", queues.present);

		stream << "Supported Extensions:" << std::endl;

		constexpr size_t groupSize = 4;
		const size_t extensionCount = extensions.size();
		for (size_t groupBegin = 0; groupBegin < extensionCount; groupBegin += groupSize) {
			const size_t groupEnd = std::min(groupBegin + groupSize, extensionCount);
			stream << "\t";
			for (size_t extensionIndex = groupBegin; extensionIndex < groupEnd; ++extensionIndex) {
				stream << extensions[extensionIndex] << ", ";
			}
			stream << std::endl;
		}
	}
}
