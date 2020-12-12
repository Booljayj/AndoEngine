#include "HAL/WindowingSystem.h"
#include "Engine/LogCommands.h"

namespace HAL {
	bool WindowingSystem::Startup(CTX_ARG) {
#if SDL_ENABLED
		mainWindow = SDL_CreateWindow("AndoSystem", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
		if (!mainWindow) {
			LOGF(SDL, Error, "Failed to create SDL window: %i", SDL_GetError());
			return false;
		}
		return true;
#else
		return false;
#endif
	}

	bool WindowingSystem::Shutdown(CTX_ARG) {
		if (mainWindow) {
#if SDL_ENABLED
			SDL_DestroyWindow(mainWindow);
#endif
		}
		mainWindow = nullptr;
		return true;
	}
}
