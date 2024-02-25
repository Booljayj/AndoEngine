#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Engine/Flags.h"
#include "Engine/Utility.h"

namespace Rendering {
	PhysicalDeviceDescription::PhysicalDeviceDescription(VkPhysicalDevice device)
		: device(device)
	{
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);
		{
			uint32_t numExtensions = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, nullptr);
			extensions.resize(numExtensions);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, extensions.data());
		}
		{
			uint32_t numFamilies = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &numFamilies, nullptr);
			t_vector<VkQueueFamilyProperties> rawFamilies{ numFamilies };
			vkGetPhysicalDeviceQueueFamilyProperties(device, &numFamilies, rawFamilies.data());

			families.resize(numFamilies);
			for (uint32_t index = 0; index < numFamilies; ++index) {
				VkQueueFamilyProperties const& properties = rawFamilies[index];

				families[index].flags = FQueueFlags::Parse(properties.queueFlags);
				families[index].count = properties.queueCount;
			}
		}
	}

	bool PhysicalDeviceDescription::SupportsExtension(char const* extension) const {
		auto const IsMatchingExtension = [extension](VkExtensionProperties const& props) { return strcmp(props.extensionName, extension) == 0; };
		return std::any_of(extensions.begin(), extensions.end(), IsMatchingExtension);
	}

	t_vector<QueueFamilyDescription> PhysicalDeviceDescription::GetSurfaceFamilies(VkSurfaceKHR surface) const {
		t_vector<QueueFamilyDescription> results;
		results.resize(families.size());
		for (uint32_t family = 0; family < families.size(); ++family) {
			results[family] = families[family];
			{
				VkBool32 hasPresentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(*this, family, surface, &hasPresentSupport);
				if (hasPresentSupport == VK_TRUE) results[family].flags += EQueueFlags::Present;
			}
		}
		return results;
	}

	std::optional<PhysicalDevicePresentation> PhysicalDevicePresentation::GetPresentation(PhysicalDeviceDescription const& physical, VkSurfaceKHR surface) {
		PhysicalDevicePresentation result;
		{
			uint32_t numFormats = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &numFormats, nullptr);
			result.surfaceFormats.resize(numFormats);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &numFormats, result.surfaceFormats.data());

			if (result.surfaceFormats.empty()) return std::optional<PhysicalDevicePresentation>{};
		}
		{
			uint32_t numModes = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &numModes, nullptr);
			result.presentModes.resize(numModes);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &numModes, result.presentModes.data());

			if (result.presentModes.empty()) return std::optional<PhysicalDevicePresentation>{};
		}

		return result;
	}

	PhysicalDeviceCapabilities::PhysicalDeviceCapabilities(VkPhysicalDevice physical, VkSurfaceKHR surface) {
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &capabilities);
	}

	uint32_t PhysicalDeviceCapabilities::GetImageCountMinimum() const {
		uint32_t const maxImageCountActual = capabilities.maxImageCount > 0 ? capabilities.maxImageCount : std::numeric_limits<uint32_t>::max();
		return std::min<uint32_t>(capabilities.minImageCount + 1, maxImageCountActual);
	}

	glm::u32vec2 PhysicalDeviceCapabilities::GetSwapExtent(VkSurfaceKHR const& surface, glm::u32vec2 const& desiredExtent) const {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return glm::u32vec2{ capabilities.currentExtent.width, capabilities.currentExtent.height };
		} else {
			glm::u32vec2 actualExtent;
			actualExtent.x = std::clamp(desiredExtent.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.y = std::clamp(desiredExtent.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}
	
	VkSurfaceTransformFlagBitsKHR PhysicalDeviceCapabilities::GetPreTransform(VkSurfaceKHR const& surface) const {
		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		} else {
			return capabilities.currentTransform;
		}
	}
}
