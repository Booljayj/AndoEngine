#pragma once
#include "Engine/ArrayView.h"
#include "Engine/Context.h"
#include "Engine/STL.h"
#include "Rendering/Vulkan/VulkanCommon.h"

struct SDL_Window;

namespace Rendering {
	/**
	 * Contains basic components for a Vulkan rendering system. These typically last for the lifetime of the application,
	 * unless fundamental rendering parameters change.
	 */
	struct VulkanFramework {
		/** The underlying vulkan instance */
		VkInstance instance = nullptr;
		/** The surface tied to the window */
		VkSurfaceKHR surface = nullptr;
		/** The oldest version of the VulkanAPI that this application requires */
		VulkanVersion version = 0;

		/** The messenger responsible for interpreting debug messages */
		VkDebugUtilsMessengerEXT messenger = nullptr;

		inline operator bool() const { return instance && surface && messenger; }

		bool Create(CTX_ARG, SDL_Window* window);
		void Destroy();

	private:
		static TArrayView<char const*> GetValidationLayerNames(CTX_ARG);
		static bool CanEnableValidationLayers(CTX_ARG, TArrayView<char const*> const& enabledLayerNames);

		static TArrayView<char const*> GetExtensionsNames(CTX_ARG, SDL_Window* window);
		static bool CanEnableExtensions(CTX_ARG, TArrayView<char const*> const& enabledExtensionNames);
	};
}
