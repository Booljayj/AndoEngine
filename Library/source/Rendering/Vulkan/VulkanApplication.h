#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "Engine/Context.h"

struct SDL_Window;

namespace Rendering {
	struct VulkanApplication {
		/** The underlying vulkan instance */
		VkInstance instance;
		/** The surface tied to the window */
		VkSurfaceKHR surface;

		bool Create(CTX_ARG, SDL_Window* window);
		void Destroy();
	};
}
