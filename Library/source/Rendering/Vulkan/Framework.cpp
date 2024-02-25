#include "Rendering/Vulkan/Framework.h"
#include "Engine/Logging.h"
#include "Engine/StringBuilding.h"

namespace Rendering {
	Framework::Framework(HAL::Window const& window) {
		ScopedThreadBufferMark mark;

#ifdef VULKAN_DEBUG
		//Information to create a debug messenger, used in several locations within this function.
		VkDebugUtilsMessengerCreateInfoEXT const messengerCI = GetDebugUtilsMessengerCreateInfo();
#endif

		//Vulkan Instance
		{
			//Layer support
			uint32_t numLayers = 0;
			vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
			layers.resize(numLayers);
			vkEnumerateInstanceLayerProperties(&numLayers, layers.data());

			auto const requiredLayers = GetInstanceLayerNames();
			for (auto layer : requiredLayers) {
				if (!SupportsLayer(layer)) throw FormatType<std::runtime_error>("Required layer %s is not supported by Vulkan instances", layer);
			}

			//Extension support
			uint32_t numExtensions = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);
			extensions.resize(numExtensions);
			vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensions.data());

			auto const requiredExtensions = GetInstanceExtensionNames(window);
			for (auto extension : requiredExtensions) {
				if (!SupportsExtension(extension)) throw FormatType<std::runtime_error>("Required extension %s is not supported by Vulkan instances", extension);
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
				.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
				.ppEnabledLayerNames = requiredLayers.data(),
				//Extensions
				.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
				.ppEnabledExtensionNames = requiredExtensions.data(),
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

			auto const requiredDeviceExtensions = GetDeviceExtensionNames();

			for (VkPhysicalDevice device : devices) {
				PhysicalDeviceDescription const physical{ device };

				for (auto extension : requiredDeviceExtensions) {
					if (!physical.SupportsExtension(extension)) {
						LOG(Vulkan, Warning, "Physical device {} cannot be used because it does not support required extension {}", physical.properties.deviceName, extension);
						continue;
					}
				}

				physicalDevices.emplace_back(physical);
			}

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

	t_vector<char const*> Framework::GetInstanceLayerNames() {
		return {
#ifdef VULKAN_DEBUG
			"VK_LAYER_KHRONOS_validation"
#endif
		};
	}

	t_vector<char const*> Framework::GetInstanceExtensionNames(HAL::Window const& window) {
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
		SDL_Vulkan_GetInstanceExtensions(window, &numHALExtensions, nullptr);
		t_vector<char const*> halExtensions{ numHALExtensions };
		SDL_Vulkan_GetInstanceExtensions(window, &numHALExtensions, halExtensions.data());

		//Create the full list of extensions that will be provided to the API
		t_vector<char const*> extensions;
		extensions.reserve(numStandardExtensions + halExtensions.size());

		extensions.insert(extensions.end(), standardExtensions, standardExtensions + numStandardExtensions);
		extensions.insert(extensions.end(), halExtensions.begin(), halExtensions.end());

		return extensions;
	}

	t_vector<char const*> Framework::GetDeviceExtensionNames() {
		return {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		};
	}

	bool Framework::SupportsLayer(char const* layer) const {
		auto const IsMatchingLayer = [layer](VkLayerProperties const& props) { return strcmp(props.layerName, layer) == 0; };
		return std::any_of(layers.begin(), layers.end(), IsMatchingLayer);
	}

	bool Framework::SupportsExtension(char const* extension) const {
		auto const IsMatchingExtension = [extension](VkExtensionProperties const& props) { return strcmp(props.extensionName, extension) == 0; };
		return std::any_of(extensions.begin(), extensions.end(), IsMatchingExtension);
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
		TemporaryStringBuilder contextBuilder;
		const auto SafeGetName = [](char const* name){
			if (name == nullptr) return "UNKNOWN";
			else return name;
		};

		//Add a list of object names referenced by this message. The first object is already part of the message.
		if (pCallbackData->objectCount > 1) {
			contextBuilder << "; Objects: "sv;

			contextBuilder << SafeGetName(pCallbackData->pObjects[1].pObjectName);
			for (size_t index = 2; index < pCallbackData->objectCount; ++index) {
				contextBuilder << ", "sv;
				contextBuilder << SafeGetName(pCallbackData->pObjects[index].pObjectName);
			}
		}
		//Add a list of queue labels referenced by this message
		if (pCallbackData->queueLabelCount > 0) {
			contextBuilder << "; Queues: "sv;

			contextBuilder << SafeGetName(pCallbackData->pQueueLabels[0].pLabelName);
			for (size_t index = 1; index < pCallbackData->queueLabelCount; ++index) {
				contextBuilder << ", "sv;
				contextBuilder << SafeGetName(pCallbackData->pQueueLabels[index].pLabelName);
			}
		}
		//Add a list of command labels referenced by this message
		if (pCallbackData->cmdBufLabelCount > 0) {
			contextBuilder << "; Command Buffers: "sv;

			contextBuilder << SafeGetName(pCallbackData->pCmdBufLabels[0].pLabelName);
			for (size_t index = 1; index < pCallbackData->cmdBufLabelCount; ++index) {
				contextBuilder << ", "sv;
				contextBuilder << SafeGetName(pCallbackData->pCmdBufLabels[index].pLabelName);
			}
		}

		//Log the message
		if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			LOG(VulkanMessage, Error, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
		} else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			LOG(VulkanMessage, Warning, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
		} else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			LOG(VulkanMessage, Info, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
		} else {
			LOG(VulkanMessage, Debug, "{}{} {}{}", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessage, contextBuilder.Get());
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
