#pragma once
#include "Engine/ArrayView.h"
#include "Engine/Context.h"
#include "Engine/LinearContainers.h"
#include "Engine/STL.h"
#include "Engine/StringBuilder.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/**
	 * Contains basic components for a Vulkan rendering system. These typically last for the lifetime of the application,
	 * unless fundamental rendering parameters change.
	 */
	struct VulkanFramework {
		/** The underlying vulkan instance */
		VkInstance instance = nullptr;
		/** The oldest version of the VulkanAPI that this application requires */
		VulkanVersion version = VK_API_VERSION_1_0;

		inline operator bool() const { return instance; }

		bool Create(CTX_ARG, HAL::Window window);
		void Destroy();

		template<typename Signature>
		Signature GetFunction(char const* name) {
			return (Signature)vkGetInstanceProcAddr(instance, name);
		}

	private:
#ifdef VULKAN_DEBUG
		PFN_vkCreateDebugUtilsMessengerEXT createMessenger = nullptr;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyMessenger = nullptr;
		/** The messenger responsible for interpreting debug messages */
		VkDebugUtilsMessengerEXT messenger = nullptr;

		/** Callback function invoked by the Vulkan API when a message should be logged */
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		/** Get the creation info struct for the debug messenger */
		static VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo();

		static l_vector<char const*> GetValidationLayerNames(CTX_ARG);
		static bool CanEnableValidationLayers(CTX_ARG, TArrayView<char const*> const& enabledLayerNames);
#endif

		static l_vector<char const*> GetExtensionsNames(CTX_ARG, HAL::Window window);
		static bool CanEnableExtensions(CTX_ARG, TArrayView<char const*> const& enabledExtensionNames);
	};
}
