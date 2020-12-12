#include "Rendering/Vulkan/VulkanDebug.h"
#include "Engine/LinearStrings.h"
#include "Engine/LogCommands.h"

DEFINE_LOG_CATEGORY(VulkanMessage, Debug);

namespace Rendering {
	using TypePrefix = std::array<char, 7>;

	TypePrefix GetTypePrefix(VkDebugUtilsMessageTypeFlagsEXT messageType) {
		TypePrefix prefix;
		size_t numCharacters = 0;

		prefix[0] = '\0';
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
			std::memcpy(prefix.data() + numCharacters, "G|", 3);
			numCharacters += 2;
		}
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
			std::memcpy(prefix.data() + numCharacters, "V|", 3);
			numCharacters += 2;
		}
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
			std::memcpy(prefix.data() + numCharacters, "P|", 3);
			numCharacters += 2;
		}

		//If we have any prefixes written, convert the trailing '|' into a space
		if (numCharacters > 0) prefix[numCharacters-1] = ' ';
		return prefix;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		Context& CTX = *static_cast<Context*>(pUserData);
		TEMP_ALLOCATOR_MARK();

		static char const* format = "%s%s";
		TypePrefix const prefix = GetTypePrefix(messageType);

		if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			LOGF(VulkanMessage, Error, format, prefix.data(), pCallbackData->pMessage);
		} else if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			LOGF(VulkanMessage, Warning, format, prefix.data(), pCallbackData->pMessage);
		} else if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			LOGF(VulkanMessage, Info, format, prefix.data(), pCallbackData->pMessage);
		} else {
			LOGF(VulkanMessage, Debug, format, prefix.data(), pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo(CTX_ARG) {
		VkDebugUtilsMessengerCreateInfoEXT messengerCI = {};
		messengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messengerCI.messageSeverity =
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messengerCI.messageType =
			//VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		messengerCI.pfnUserCallback = &Rendering::VulkanDebugCallback;
		messengerCI.pUserData = &CTX;

		return messengerCI;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    	auto const func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto const func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func) func(instance, debugMessenger, pAllocator);
	}
}
