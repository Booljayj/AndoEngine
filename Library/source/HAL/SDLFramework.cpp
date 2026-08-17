#include "HAL/SDLFramework.h"
#include "Engine/Format.h"
#include "Engine/Logging.h"
#include "ThirdParty/SDL2.h"

namespace HAL {
	SDLFramework::SDLFramework() {
		LOG(Application, Info, "Starting SDL framework");
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			throw FormatType<std::runtime_error>("SDL_Init Error: {}", SDL_GetError());
		}
	}

	SDLFramework::~SDLFramework() {
		LOG(Application, Info, "Stopping SDL framework");
		SDL_Quit();
	}
}
