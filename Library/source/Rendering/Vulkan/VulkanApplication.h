#pragma once
#include <string>
#include "Engine/ArrayView.h"
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanCommon.h"

struct SDL_Window;

namespace Rendering {
	struct VulkanApplication {
		/** The underlying vulkan instance */
		VkInstance instance;
		/** The surface tied to the window */
		VkSurfaceKHR surface;

		/** The messenger responsible for interpreting debug messages */
		VkDebugUtilsMessengerEXT messenger;

		bool Create(CTX_ARG, SDL_Window* window);
		void Destroy();

	private:
		static TArrayView<char const*> GetValidationLayerNames(CTX_ARG);
		static bool CanEnableValidationLayers(CTX_ARG, TArrayView<char const*> const& enabledLayerNames);

		static TArrayView<char const*> GetExtensionsNames(CTX_ARG, SDL_Window* window);
		static bool CanEnableExtensions(CTX_ARG, TArrayView<char const*> const& enabledExtensionNames);
	};
}
