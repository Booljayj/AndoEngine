#include <SDL2/SDL_vulkan.h>
#include "Engine/ScopedTempBlock.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanApplication.h"

namespace Rendering {
	bool VulkanApplication::Create(CTX_ARG, SDL_Window* window) {
		TEMP_SCOPE;

		// Vulkan Instance
		{
			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			//@todo Get this information from a common, configurable location
			appInfo.pApplicationName = "DefaultProject";
			appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
			//@todo Get this information from a common location
			appInfo.pEngineName = "AndoEngine";
			appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);

			appInfo.apiVersion = VK_API_VERSION_1_0;

			uint32_t sdlExtensionCount = 0;
			SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);
			char const** sdlExtensionNames = CTX.temp.Request<char const*>(sdlExtensionCount);
			SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, sdlExtensionNames);

			VkInstanceCreateInfo instanceCI = {};
			instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instanceCI.pApplicationInfo = &appInfo;
			instanceCI.enabledExtensionCount = sdlExtensionCount;
			instanceCI.ppEnabledExtensionNames = sdlExtensionNames;

			if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS) {
				LOG(LogTemp, Error, "Failed to create Vulkan instance");
				return false;
			}
		}

		// Vulkan surface (via SDL)
		{
			if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE) {
				LOG(LogTemp, Error, "Failed to create Vulkan window surface");
				return false;
			}
		}

		return true;
	}

	void VulkanApplication::Destroy() {
		if (!!surface) vkDestroySurfaceKHR(instance, surface, nullptr);
		if (!!instance) vkDestroyInstance(instance, nullptr);
	}
}