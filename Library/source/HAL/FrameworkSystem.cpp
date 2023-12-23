#include "HAL/FrameworkSystem.h"
#include "Engine/Logging.h"
#include "HAL/SDL2.h"

namespace HAL {
	bool FrameworkSystem::Startup() {
		if (SDL_Init(SDL_INIT_VIDEO) == 0) {
			return true;
		} else {
			LOG(SDL, Error, "SDL_Init Error: {}", SDL_GetError());
			return false;
		}
	}

	bool FrameworkSystem::Shutdown() {
		SDL_Quit();
		return true;
	}
}
