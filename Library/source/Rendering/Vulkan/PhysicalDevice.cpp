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

	PhysicalDeviceDescription::PhysicalDeviceDescription(VkPhysicalDevice device)
		: device(device)
		, supported_features(device)
	{
		vkGetPhysicalDeviceProperties(device, &properties);
		{
			uint32_t num_extensions = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &num_extensions, nullptr);
			supported_extensions.resize(num_extensions);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &num_extensions, supported_extensions.data());
		}
		{
			uint32_t num_families = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &num_families, nullptr);
			t_vector<VkQueueFamilyProperties> raw_families{ num_families };
			vkGetPhysicalDeviceQueueFamilyProperties(device, &num_families, raw_families.data());

			families.resize(num_families);
			for (uint32_t index = 0; index < num_families; ++index) {
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
