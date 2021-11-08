#include "HAL/FrameworkSystem.h"
#include "Engine/LogCommands.h"
#include "HAL/SDL2.h"

namespace HAL {
	bool FrameworkSystem::Startup() {
#if SDL_ENABLED
		if (SDL_Init(SDL_INIT_VIDEO) == 0) {
			return true;
		} else {
			LOGF(SDL, Error, "SDL_Init Error: %i", SDL_GetError());
			return false;
		}
#else
		return false;
#endif
	}

	bool FrameworkSystem::Shutdown() {
#if SDL_ENABLED
		SDL_Quit();
#endif
		return true;
	}
}
