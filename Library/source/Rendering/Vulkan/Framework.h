#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/TemporaryContainers.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct Environment;

	/**
	 * Contains basic components for a Vulkan rendering system. These typically last for the lifetime of the application,
	 * unless fundamental rendering parameters change.
	 */
	struct Framework {
		using NameSpan = std::span<char const* const>;

		Framework(Environment const& environment, NameSpan enabled_layer_names, NameSpan enabled_extension_names);
		Framework(Framework const&) = delete;
		Framework(Framework&&) = delete;
		~Framework();

		inline operator VkInstance() const { return instance; }
		
		/** Get the oldest version of the Vulkan API that this application requires */
		static VulkanVersion GetMinVersion() { return VK_API_VERSION_1_2; }

		/** Get a pointer to a Vulkan API function */
		template<typename Signature>
		Signature GetFunction(char const* name) const { return (Signature)vkGetInstanceProcAddr(instance, name); }

		/** Get the physical devices available on this machine */
		std::vector<PhysicalDeviceDescription> const& GetPhysicalDevices() const { return physicalDevices; }

	private:
		VkInstance instance = nullptr;

		/** Physical devices which can be used */
		std::vector<PhysicalDeviceDescription> physicalDevices;

#ifdef VULKAN_DEBUG
		PFN_vkCreateDebugUtilsMessengerEXT createMessenger = nullptr;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyMessenger = nullptr;
		/** The messenger responsible for interpreting debug messages */
		VkDebugUtilsMessengerEXT messenger = nullptr;
#endif

#ifdef VULKAN_DEBUG
		/** Callback function invoked by the Vulkan API when a message should be logged */
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		/** Get the creation info struct for the debug messenger */
		static VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo();
#endif

	};
}
