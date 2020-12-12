#pragma once
#include "Engine/Context.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Callback function invoked by the Vulkan API when a message should be logged */
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	/** Get the creation info struct for the debug messenger */
	VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo(CTX_ARG);
	/** Vulkan Extension: Create a VkDebugUtilsMessengerEXT */
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	/** Vulkan Extension: Destory a VkDebugUtilsMessengerEXT */
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
}
