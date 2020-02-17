#include <SDL2/SDL_vulkan.h>
#include "Engine/ScopedTempBlock.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanApplication.h"
#include "Rendering/Vulkan/VulkanDebug.h"

namespace Rendering {
	bool VulkanApplication::Create(CTX_ARG, SDL_Window* window) {
		TEMP_SCOPE;

		//Information to create a debug messenger, used in several locations within this function.
		VkDebugUtilsMessengerCreateInfoEXT const messengerCI = Rendering::GetDebugUtilsMessengerCreateInfo(CTX);

		// Vulkan Instance
		{
			//Check for validation layer support
			TArrayView<char const*> const enabledLayerNames = GetValidationLayerNames(CTX);
			if (!CanEnableValidationLayers(CTX, enabledLayerNames)) {
				LOG(LogVulkan, Error, "Cannot enable required validation layers");
				return false;
			}

			//Check for extension support
			TArrayView<char const*> const enabledExtensionNames = GetExtensionsNames(CTX, window);
			if (!CanEnableExtensions(CTX, enabledExtensionNames)) {
				LOG(LogVulkan, Error, "Cannot enable required instance extensions");
				return false;
			}

			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			//Application info
			//@todo Get this information from a common, configurable location
			appInfo.pApplicationName = "DefaultProject";
			appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
			//Engine info
			//@todo Get this information from a common location
			appInfo.pEngineName = "AndoEngine";
			appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
			//Vulkan info
			appInfo.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo instanceCI = {};
			instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instanceCI.pApplicationInfo = &appInfo;
			//Extensions
			instanceCI.enabledExtensionCount = enabledExtensionNames.size();
			instanceCI.ppEnabledExtensionNames = enabledExtensionNames.begin();

			//Validation layers
			instanceCI.enabledLayerCount = enabledLayerNames.size();
			instanceCI.ppEnabledLayerNames = enabledLayerNames.begin();
			//Debug messenger for messages that are sent during instance creation
			instanceCI.pNext = &messengerCI;

			if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS) {
				LOG(LogVulkan, Error, "Failed to create Vulkan instance");
				return false;
			}
		}

		// Vulkan surface (via SDL)
		{
			if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE) {
				LOG(LogVulkan, Error, "Failed to create Vulkan window surface");
				return false;
			}
		}

		// Vulkan Messenger
		{
			if (CreateDebugUtilsMessengerEXT(instance, &messengerCI, nullptr, &messenger) != VK_SUCCESS) {
				LOG(LogVulkan, Error, "Failed to create debug messenger");
				return false;
			}
		}

		return true;
	}

	void VulkanApplication::Destroy() {
		if (!!messenger) DestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
		if (!!surface) vkDestroySurfaceKHR(instance, surface, nullptr);
		if (!!instance) vkDestroyInstance(instance, nullptr);
	}

	TArrayView<char const*> VulkanApplication::GetValidationLayerNames(CTX_ARG) {
		constexpr uint32_t enabledLayerCount = 1;
		static char const* enabledLayerNames[enabledLayerCount] = {"VK_LAYER_KHRONOS_validation"};
		return TArrayView<char const*>{enabledLayerNames, enabledLayerCount};
	}

	bool VulkanApplication::CanEnableValidationLayers(CTX_ARG, TArrayView<char const*> const& enabledLayerNames) {
		TEMP_SCOPE;

		uint32_t availableLayerCount;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
		VkLayerProperties* availableLayers = CTX.temp.Request<VkLayerProperties>(availableLayerCount);
		vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

		for (char const* enabledLayerName : enabledLayerNames) {
			bool layerFound = false;
			for (uint32_t availableIndex = 0; availableIndex < availableLayerCount; ++availableIndex) {
				VkLayerProperties const& availableLayer = availableLayers[availableIndex];
				if (strcmp(availableLayer.layerName, enabledLayerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) return false;
		}
		return true;
	}

	TArrayView<char const*> VulkanApplication::GetExtensionsNames(CTX_ARG, SDL_Window* window) {
		constexpr uint32_t debugExtensionCount = 1;
		static char const* debugExtensionNames[debugExtensionCount] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

		uint32_t sdlExtensionCount = 0;
		SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);

		const size_t extensionCount = debugExtensionCount + sdlExtensionCount;
		char const** extensionNames = CTX.temp.Request<char const*>(extensionCount);

		//Append the standard and SDL extensions together into the destination array
		std::memcpy(extensionNames, debugExtensionNames, (debugExtensionCount * sizeof(char const*)));
		SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, extensionNames + debugExtensionCount);

		return TArrayView<char const*>{extensionNames, extensionCount};
	}

	bool VulkanApplication::CanEnableExtensions(CTX_ARG, TArrayView<char const*> const& enabledExtensionNames) {
		TEMP_SCOPE;

		uint32_t availableExtensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		VkExtensionProperties* availableExtensions = CTX.temp.Request<VkExtensionProperties>(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions);

		for (char const* enabledExtensionName : enabledExtensionNames) {
			bool extensionFound = false;
			for (size_t availableIndex = 0; availableIndex < availableExtensionCount; ++availableIndex) {
				VkExtensionProperties const& availableExtension = availableExtensions[availableIndex];
				if (strcmp(availableExtension.extensionName, enabledExtensionName) == 0) {
					extensionFound = true;
					break;
				}
			}
			if (!extensionFound) return false;
		}
		return true;
	}
}
