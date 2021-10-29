#include "Rendering/Vulkan/VulkanFramework.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanDebug.h"

namespace Rendering {
	bool VulkanFramework::Create(CTX_ARG, HAL::Window window) {
		TEMP_ALLOCATOR_MARK();

		//Information to create a debug messenger, used in several locations within this function.
		VkDebugUtilsMessengerCreateInfoEXT const messengerCI = Rendering::GetDebugUtilsMessengerCreateInfo(CTX);

		// Vulkan Instance
		{
			//Check for validation layer support
			TArrayView<char const*> const enabledLayerNames = GetValidationLayerNames(CTX);
			if (!CanEnableValidationLayers(CTX, enabledLayerNames)) {
				LOG(Vulkan, Error, "Cannot enable required validation layers");
				return false;
			}

			//Check for extension support
			TArrayView<char const*> const enabledExtensionNames = GetExtensionsNames(CTX, window);
			if (!CanEnableExtensions(CTX, enabledExtensionNames)) {
				LOG(Vulkan, Error, "Cannot enable required instance extensions");
				return false;
			}

			version = VK_API_VERSION_1_0;

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
			appInfo.apiVersion = version;

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

			assert(!instance);
			if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to create Vulkan instance");
				return false;
			}
		}

		// Vulkan Messenger
		assert(!messenger);
		if (CreateDebugUtilsMessengerEXT(instance, &messengerCI, nullptr, &messenger) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create debug messenger");
			return false;
		}

		return true;
	}

	void VulkanFramework::Destroy() {
		if (messenger) DestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
		if (instance) vkDestroyInstance(instance, nullptr);

		messenger = nullptr;
		instance = nullptr;
	}

	TArrayView<char const*> VulkanFramework::GetValidationLayerNames(CTX_ARG) {
		constexpr uint32_t enabledLayerCount = 1;
		static char const* enabledLayerNames[enabledLayerCount] = {"VK_LAYER_KHRONOS_validation"};
		return TArrayView<char const*>{enabledLayerNames, enabledLayerCount};
	}

	bool VulkanFramework::CanEnableValidationLayers(CTX_ARG, TArrayView<char const*> const& enabledLayerNames) {
		TEMP_ALLOCATOR_MARK();

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

	TArrayView<char const*> VulkanFramework::GetExtensionsNames(CTX_ARG, HAL::Window window) {
		//Standard extensions which the application requires
		constexpr char const* standardExtensions[] = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		};
		constexpr size_t numStandardExtensions = std::size(standardExtensions);

		uint32_t numHALExtensions = 0;
		char const** halExtensions = nullptr;
#if SDL_ENABLED
		//Extensions needed by SDL
		SDL_Vulkan_GetInstanceExtensions(window.handle, &numHALExtensions, nullptr);
		halExtensions = CTX.temp.Request<char const*>(numHALExtensions);
		SDL_Vulkan_GetInstanceExtensions(window.handle, &numHALExtensions, halExtensions);
#endif

		//Create the full list of extensions that will be provided to the API
		const size_t numExtensions = numStandardExtensions + numHALExtensions;
		char const** extensions = CTX.temp.Request<char const*>(numExtensions);

		std::memcpy(
			extensions,
			standardExtensions, numStandardExtensions * sizeof(char const*)
		);
		std::memcpy(
			extensions + numStandardExtensions,
			halExtensions, numHALExtensions * sizeof(char const*)
		);

		return TArrayView<char const*>{extensions, numExtensions};
	}

	bool VulkanFramework::CanEnableExtensions(CTX_ARG, TArrayView<char const*> const& enabledExtensionNames) {
		TEMP_ALLOCATOR_MARK();

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
			if (!extensionFound) {
				LOGF(Vulkan, Warning, "Cannot enable extension: %s", enabledExtensionName);
				return false;
			}
		}
		return true;
	}
}
