#pragma once
#include "HAL/Platform.h"

#if PLATFORM_IS_DESKTOP
#define SDL_ENABLED 1
#else
#define SDL_ENABLED 0
#endif

#if SDL_ENABLED
#include <SDL2/SDL.h>
#include "Engine/Logging.h"

DECLARE_LOG_CATEGORY(SDL);
#endif
