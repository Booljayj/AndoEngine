#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Engine/Flags.h"
#include "Engine/Utility.h"

namespace Rendering {
	PhysicalDeviceFeatures::PhysicalDeviceFeatures() {
		version10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		version10.pNext = &version11;

		version11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		version11.pNext = &version12;

		version12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		version12.pNext = nullptr;
	}

	PhysicalDeviceFeatures::PhysicalDeviceFeatures(VkPhysicalDevice device)
		: PhysicalDeviceFeatures()
	{
		vkGetPhysicalDeviceFeatures2(device, &version10);
	}

	PhysicalDeviceFeatures::PhysicalDeviceFeatures(const PhysicalDeviceFeatures& other) {
		version10 = other.version10;
		version11 = other.version11;
		version12 = other.version12;

		version10.pNext = &version11;
		version11.pNext = &version12;
		version12.pNext = nullptr;
	}

	PhysicalDeviceCapabilities::PhysicalDeviceCapabilities(VkPhysicalDevice physical, VkSurfaceKHR surface) {
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &capabilities);
	}

	uint32_t PhysicalDeviceCapabilities::GetImageCountMinimum() const {
		uint32_t const maxImageCountActual = capabilities.maxImageCount > 0 ? capabilities.maxImageCount : std::numeric_limits<uint32_t>::max();
		return std::min<uint32_t>(capabilities.minImageCount + 1, maxImageCountActual);
	}

	glm::u32vec2 PhysicalDeviceCapabilities::GetSwapExtent(glm::u32vec2 const& desiredExtent) const {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return glm::u32vec2{ capabilities.currentExtent.width, capabilities.currentExtent.height };
		} else {
			glm::u32vec2 actualExtent;
			actualExtent.x = std::clamp(desiredExtent.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.y = std::clamp(desiredExtent.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}

	VkSurfaceTransformFlagBitsKHR PhysicalDeviceCapabilities::GetPreTransform() const {
		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else {
			return capabilities.currentTransform;
		}
	}

	PhysicalDeviceDescription::PhysicalDeviceDescription(VkPhysicalDevice device)
		: device(device)
		, supported_features(device)
	{
		vkGetPhysicalDeviceProperties(device, &properties);
		
		supported_extensions = GetResults<std::vector<VkExtensionProperties>>(&vkEnumerateDeviceExtensionProperties, device, nullptr);

		{
			t_vector<VkQueueFamilyProperties> raw_families = GetResults<t_vector<VkQueueFamilyProperties>>(&vkGetPhysicalDeviceQueueFamilyProperties, device);
			
			families.resize(raw_families.size());
			for (uint32_t index = 0; index < raw_families.size(); ++index) {
				VkQueueFamilyProperties const& family = raw_families[index];

				families[index].flags = FQueueFlags::Create(family.queueFlags);
				families[index].size = family.queueCount;
			}
		}
	}

	bool PhysicalDeviceDescription::SupportsExtension(char const* extension_name) const {
		auto const MatchesExtensionName = [extension_name](VkExtensionProperties const& extension) { return strcmp(extension.extensionName, extension_name) == 0; };
		return ranges::any_of(supported_extensions, MatchesExtensionName);
	}

	PhysicalDeviceCapabilities PhysicalDeviceDescription::GetSurfaceCapabilities(VkSurfaceKHR surface) const
	{
		return PhysicalDeviceCapabilities{ device, surface };
	}

	std::vector<QueueFamilyDescription> PhysicalDeviceDescription::GetSurfaceQueueFamilies(VkSurfaceKHR surface) const {
		std::vector<QueueFamilyDescription> results;
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

	std::vector<VkSurfaceFormatKHR> PhysicalDeviceDescription::GetSurfaceFormats(VkSurfaceKHR surface) const {
		return GetResults<std::vector<VkSurfaceFormatKHR>>(&vkGetPhysicalDeviceSurfaceFormatsKHR, *this, surface);
	}

	std::vector<VkPresentModeKHR> PhysicalDeviceDescription::GetSurfacePresentModes(VkSurfaceKHR surface) const {
		return GetResults<std::vector<VkPresentModeKHR>>(&vkGetPhysicalDeviceSurfacePresentModesKHR, *this, surface);
	}
}
