#include "Rendering/Vulkan/VulkanFramework.h"
#include "Engine/LogCommands.h"

namespace Rendering {
	bool VulkanFramework::Create(CTX_ARG, HAL::Window window) {
		TEMP_ALLOCATOR_MARK();

#ifdef VULKAN_DEBUG
		//Information to create a debug messenger, used in several locations within this function.
		VkDebugUtilsMessengerCreateInfoEXT const messengerCI = GetDebugUtilsMessengerCreateInfo();
#endif

		// Vulkan Instance
		{
#ifdef VULKAN_DEBUG
			//Check for validation layer support
			l_vector<char const*> const enabledLayerNames = GetValidationLayerNames(CTX);
			if (!CanEnableValidationLayers(CTX, enabledLayerNames)) {
				LOG(Vulkan, Error, "Cannot enable required validation layers");
				return false;
			}
#endif

			//Check for extension support
			l_vector<char const*> const enabledExtensionNames = GetExtensionsNames(CTX, window);
			if (!CanEnableExtensions(CTX, enabledExtensionNames)) {
				LOG(Vulkan, Error, "Cannot enable required instance extensions");
				return false;
			}

			version = VK_API_VERSION_1_0;

			//Application info
			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			//@todo Get this information from a common, configurable location
			appInfo.pApplicationName = "DefaultProject";
			appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
			//Engine info
			//@todo Get this information from a common location
			appInfo.pEngineName = "AndoEngine";
			appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
			//Vulkan info
			appInfo.apiVersion = version;

			VkInstanceCreateInfo instanceCI{};
			instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instanceCI.pApplicationInfo = &appInfo;
			//Extensions
			instanceCI.enabledExtensionCount = enabledExtensionNames.size();
			instanceCI.ppEnabledExtensionNames = enabledExtensionNames.data();
#ifdef VULKAN_DEBUG
			//Validation layers
			instanceCI.enabledLayerCount = enabledLayerNames.size();
			instanceCI.ppEnabledLayerNames = enabledLayerNames.data();
			//Debug messenger for messages that are sent during instance creation
			instanceCI.pNext = &messengerCI;
#endif

			assert(!instance);
			if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to create Vulkan instance");
				return false;
			}
		}

#ifdef VULKAN_DEBUG
		// Vulkan Messenger
		createMessenger = GetFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT");
		destroyMessenger = GetFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT");
		if (!createMessenger || !destroyMessenger) {
			LOG(Vulkan, Error, "Failed to find debug messenger extension methods");
			return false;
		}

		assert(!messenger);
		if (createMessenger(instance, &messengerCI, nullptr, &messenger) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create debug messenger");
			return false;
		}
#endif

		return true;
	}

	void VulkanFramework::Destroy() {
#ifdef VULKAN_DEBUG
		if (messenger) destroyMessenger(instance, messenger, nullptr);
		messenger = nullptr;
		createMessenger = nullptr;
		destroyMessenger = nullptr;
#endif

		if (instance) vkDestroyInstance(instance, nullptr);
		instance = nullptr;
		version = VK_API_VERSION_1_0;
	}

#ifdef VULKAN_DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanFramework::VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		TEMP_ALLOCATOR_MARK();

		//Create a prefix based on the message type flags
		std::string_view prefix;
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
			prefix = "G "sv;
		}
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
			prefix = "V "sv;
		}
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
			prefix = "P "sv;
		}

		//Create a string that provides additional contextual information
		StringBuilder contextBuilder;
		//Add a list of object names referenced by this message. The first object is already part of the message.
		if (pCallbackData->objectCount > 1) {
			contextBuilder << "; Objects: "sv;

			contextBuilder << pCallbackData->pObjects[1].pObjectName;
			for (size_t index = 2; index < pCallbackData->objectCount; ++index) {
				contextBuilder << ", "sv;
				contextBuilder << pCallbackData->pObjects[index].pObjectName;
			}
		}
		//Add a list of queue labels referenced by this message
		if (pCallbackData->queueLabelCount > 0) {
			contextBuilder << "; Queues: "sv;

			contextBuilder << pCallbackData->pQueueLabels[0].pLabelName;
			for (size_t index = 1; index < pCallbackData->queueLabelCount; ++index) {
				contextBuilder << ", "sv;
				contextBuilder << pCallbackData->pQueueLabels[index].pLabelName;
			}
		}
		//Add a list of command labels referenced by this message
		if (pCallbackData->cmdBufLabelCount > 0) {
			contextBuilder << "; Command Buffers: "sv;

			contextBuilder << pCallbackData->pCmdBufLabels[0].pLabelName;
			for (size_t index = 1; index < pCallbackData->cmdBufLabelCount; ++index) {
				contextBuilder << ", "sv;
				contextBuilder << pCallbackData->pCmdBufLabels[index].pLabelName;
			}
		}

		//Log the message
		if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			LOGF(VulkanMessage, Error, "%s%i %s%s", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
		} else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			LOGF(VulkanMessage, Warning, "%s%i %s%s", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
		} else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			LOGF(VulkanMessage, Info, "%s%i %s%s", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
		} else {
			LOGF(VulkanMessage, Debug, "%s%i %s%s", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
		}

		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT VulkanFramework::GetDebugUtilsMessengerCreateInfo() {
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
		messengerCI.pfnUserCallback = &VulkanFramework::VulkanDebugCallback;

		return messengerCI;
	}

	l_vector<char const*> VulkanFramework::GetValidationLayerNames(CTX_ARG) {
		return l_vector<char const*>{ "VK_LAYER_KHRONOS_validation" };
	}

	bool VulkanFramework::CanEnableValidationLayers(CTX_ARG, TArrayView<char const*> const& enabledLayerNames) {
		TEMP_ALLOCATOR_MARK();

		uint32_t availableLayerCount;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
		VkLayerProperties* availableLayers = threadHeapBuffer->Request<VkLayerProperties>(availableLayerCount);
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
#endif

	l_vector<char const*> VulkanFramework::GetExtensionsNames(CTX_ARG, HAL::Window window) {
		//Standard extensions which the application requires
		constexpr char const* standardExtensions[] = {
#ifdef VULKAN_DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		};
		constexpr size_t numStandardExtensions = std::size(standardExtensions);

		//Extensions required by the HAL
		uint32_t numHALExtensions = 0;
		char const** halExtensions = nullptr;
#if SDL_ENABLED
		//Extensions needed by SDL
		SDL_Vulkan_GetInstanceExtensions(window.handle, &numHALExtensions, nullptr);
		halExtensions = threadHeapBuffer->Request<char const*>(numHALExtensions);
		SDL_Vulkan_GetInstanceExtensions(window.handle, &numHALExtensions, halExtensions);
#endif

		//Create the full list of extensions that will be provided to the API
		l_vector<char const*> extensions;
		extensions.reserve(numStandardExtensions + numHALExtensions);

		extensions.insert(extensions.end(), standardExtensions, standardExtensions + numStandardExtensions);
		extensions.insert(extensions.end(), halExtensions, halExtensions + numHALExtensions);

		return extensions;
	}

	bool VulkanFramework::CanEnableExtensions(CTX_ARG, TArrayView<char const*> const& enabledExtensionNames) {
		TEMP_ALLOCATOR_MARK();

		uint32_t availableExtensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		VkExtensionProperties* availableExtensions = threadHeapBuffer->Request<VkExtensionProperties>(availableExtensionCount);
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
