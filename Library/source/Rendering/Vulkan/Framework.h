#pragma once
#include "Engine/ArrayView.h"
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/**
	 * Contains basic components for a Vulkan rendering system. These typically last for the lifetime of the application,
	 * unless fundamental rendering parameters change.
	 */
	struct Framework {
		Framework(HAL::Window const& window);
		Framework(Framework const&) = delete;
		Framework(Framework&&) = delete;
		~Framework();

		inline operator VkInstance() const { return instance; }
		
		/** Get the oldest version of the Vulkan API that this application requires */
		static VulkanVersion GetMinVersion() { return VK_API_VERSION_1_0; }

		/** Get a pointer to a Vulkan API function */
		template<typename Signature>
		Signature GetFunction(char const* name) const { return (Signature)vkGetInstanceProcAddr(instance, name); }

		/** Get the physical devices available on this machine */
		std::vector<PhysicalDeviceDescription> const& GetPhysicalDevices() const { return physicalDevices; }

		static t_vector<char const*> GetDeviceExtensionNames();

	private:
		VkInstance instance = nullptr;

		/** Layers that are supported by this instance */
		std::vector<VkLayerProperties> layers;
		/** Extensions that are supported by this instance */
		std::vector<VkExtensionProperties> extensions;
		/** Physical devices which can be used */
		std::vector<PhysicalDeviceDescription> physicalDevices;

#ifdef VULKAN_DEBUG
		PFN_vkCreateDebugUtilsMessengerEXT createMessenger = nullptr;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyMessenger = nullptr;
		/** The messenger responsible for interpreting debug messages */
		VkDebugUtilsMessengerEXT messenger = nullptr;
#endif

		static t_vector<char const*> GetInstanceLayerNames();
		static t_vector<char const*> GetInstanceExtensionNames(HAL::Window const& window);

		bool SupportsLayer(char const* layer) const;
		bool SupportsExtension(char const* extension) const;

#ifdef VULKAN_DEBUG
		/** Callback function invoked by the Vulkan API when a message should be logged */
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		/** Get the creation info struct for the debug messenger */
		static VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo();
#endif

	};
}
