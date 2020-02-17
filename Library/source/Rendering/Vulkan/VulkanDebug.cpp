#include "Engine/LinearStrings.h"
#include "Engine/LogCommands.h"
#include "Engine/ScopedTempBlock.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanDebug.h"

namespace Rendering {
	char const* GetTypePrefix(CTX_ARG, VkDebugUtilsMessageTypeFlagsEXT messageType) {
		char* Begin = CTX.temp.Request<char>(7); //Request only enough to hold the maximum possible prefix
		size_t Characters = 0;

		Begin[0] = '\0';
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
			std::memcpy(Begin + Characters, "G|", 3);
			Characters += 2;
		}
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
			std::memcpy(Begin + Characters, "V|", 3);
			Characters += 2;
		}
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
			std::memcpy(Begin + Characters, "P|", 3);
			Characters += 2;
		}

		//If we have any prefixes written, convert the trailing '|' into a space
		if (Characters > 0) {
			Begin[Characters-1] = ' ';
		}
		return Begin;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		Context& CTX = *static_cast<Context*>(pUserData);
		TEMP_SCOPE;

		static char const* format = "%s%s";
		char const* prefix = GetTypePrefix(CTX, messageType);

		if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			LOGF(LogVulkanMessage, Error, format, prefix, pCallbackData->pMessage);
		} else if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			LOGF(LogVulkanMessage, Warning, format, prefix, pCallbackData->pMessage);
		} else if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			LOGF(LogVulkanMessage, Info, format, prefix, pCallbackData->pMessage);
		} else {
			LOGF(LogVulkanMessage, Debug, format, prefix, pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo(CTX_ARG) {
		VkDebugUtilsMessengerCreateInfoEXT messengerCI = {};
		messengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messengerCI.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
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

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	) {
    	auto const func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		} else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto const func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
}
