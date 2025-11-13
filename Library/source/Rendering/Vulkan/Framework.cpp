#include "Rendering/Vulkan/Framework.h"
#include "Rendering/Vulkan/Environment.h"
#include "Engine/Format.h"
#include "Engine/Logging.h"

namespace Rendering {
	Framework::Framework(Environment const& environment, NameSpan enabled_layer_names, NameSpan enabled_extension_names) {
		ScopedThreadBufferMark mark;

#ifdef VULKAN_DEBUG
		//Information to create a debug messenger, used in several locations within this function.
		VkDebugUtilsMessengerCreateInfoEXT const messengerCI = GetDebugUtilsMessengerCreateInfo();
#endif

		//Vulkan Instance
		{
			for (char const* layer_name : enabled_layer_names) {
				if (!environment.SupportsLayer(layer_name)) throw FormatType<std::runtime_error>("Enabled layer {} is not supported by Vulkan environment", layer_name);
			}
			for (char const* extension_name : enabled_extension_names) {
				if (!environment.SupportsExtension(extension_name)) throw FormatType<std::runtime_error>("Enabled extension {} is not supported by Vulkan environment", extension_name);
			}

			//Application info
			VkApplicationInfo const appInfo{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				//@todo Get this information from a common, configurable location
				.pApplicationName = "DefaultProject",
				.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
				//Engine info
				//@todo Get this information from a common location
				.pEngineName = "AndoEngine",
				.engineVersion = VK_MAKE_VERSION(0, 1, 0),
				//Vulkan info
				.apiVersion = GetMinVersion(),
			};
			
			VkInstanceCreateInfo const instanceCI{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#ifdef VULKAN_DEBUG
				//Debug messenger for messages that are sent during instance creation
				.pNext = &messengerCI,
#endif
				.pApplicationInfo = &appInfo,
				//Validation layers
				.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size()),
				.ppEnabledLayerNames = enabled_layer_names.data(),
				//Extensions
				.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size()),
				.ppEnabledExtensionNames = enabled_extension_names.data(),
			};
			
			if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS || !instance) {
				throw FormatType<std::runtime_error>("Failed to create Vulkan instance");
			}
		}

		//Gather physical devices which can be used
		{
			uint32_t numDevices = 0;
			vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
			t_vector<VkPhysicalDevice> devices{ numDevices };
			vkEnumeratePhysicalDevices(instance, &numDevices, devices.data());

			for (VkPhysicalDevice device : devices) physicalDevices.emplace_back(device);

			if (physicalDevices.empty()) {
				throw FormatType<std::runtime_error>("No usable physical devices were found");
			}
		}

#ifdef VULKAN_DEBUG
		// Vulkan Messenger
		createMessenger = GetFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT");
		destroyMessenger = GetFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT");
		if (!createMessenger || !destroyMessenger) {
			throw FormatType<std::runtime_error>("Failed to find debug messenger extension methods");
		}

		if (createMessenger(instance, &messengerCI, nullptr, &messenger) != VK_SUCCESS || !messenger) {
			throw FormatType<std::runtime_error>("Failed to create debug messenger");
		}
#endif
	}

	Framework::~Framework() {
#ifdef VULKAN_DEBUG
		destroyMessenger(instance, messenger, nullptr);
#endif
		vkDestroyInstance(instance, nullptr);
	}

#ifdef VULKAN_DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL Framework::VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		ScopedThreadBufferMark mark;

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
		std::string context;
		context.reserve(64);

		const auto SafeGetName = [](char const* name) { return (name == nullptr) ? "Unknown" : name; };

		//Add a list of object names referenced by this message. The first object is already part of the message.
		if (pCallbackData->objectCount > 1) {
			std::format_to(std::back_inserter(context), "; Objects: {}", SafeGetName(pCallbackData->pObjects[1].pObjectName));
			for (size_t index = 2; index < pCallbackData->objectCount; ++index) {
				std::format_to(std::back_inserter(context), ", {}", SafeGetName(pCallbackData->pObjects[index].pObjectName));
			}
		}
		//Add a list of queue labels referenced by this message
		if (pCallbackData->queueLabelCount > 0) {
			std::format_to(std::back_inserter(context), "; Queues: {}", SafeGetName(pCallbackData->pQueueLabels[0].pLabelName));
			for (size_t index = 1; index < pCallbackData->queueLabelCount; ++index) {
				std::format_to(std::back_inserter(context), ", {}", SafeGetName(pCallbackData->pQueueLabels[index].pLabelName));
			}
		}
		//Add a list of command labels referenced by this message
		if (pCallbackData->cmdBufLabelCount > 0) {
			std::format_to(std::back_inserter(context), "; Command Buffers: {}", SafeGetName(pCallbackData->pCmdBufLabels[0].pLabelName));
			for (size_t index = 1; index < pCallbackData->cmdBufLabelCount; ++index) {
				std::format_to(std::back_inserter(context), ", {}", SafeGetName(pCallbackData->pCmdBufLabels[index].pLabelName));
			}
		}

		//Log the message
		if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			LOG(VulkanMessage, Error, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, context);
			__debugbreak();
		} else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			LOG(VulkanMessage, Warning, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, context);
		} else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			LOG(VulkanMessage, Info, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, context);
		} else {
			LOG(VulkanMessage, Debug, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, context);
		}

		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT Framework::GetDebugUtilsMessengerCreateInfo() {
		return VkDebugUtilsMessengerCreateInfoEXT{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity =
				//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType =
				//VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = &Framework::VulkanDebugCallback,
		};
	}
#endif
}
