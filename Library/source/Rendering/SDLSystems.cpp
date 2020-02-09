#include <GL/glew.h>
#include "Rendering/SDLSystems.h"
#include "Engine/Context.h"
#include "Engine/Logging/LogCategory.h"
#include "Engine/LogCommands.h"
#include "Engine/LinearStrings.h"
#include "UI/IMGUI/imgui.h"
#include "UI/IMGUI/imgui_impl_sdl.h"
#include "UI/IMGUI/imgui_impl_opengl3.h"

DEFINE_LOG_CATEGORY(SDL, Debug);

bool SDLFrameworkSystem::Startup(CTX_ARG) {
	if (SDL_Init(SDL_INIT_VIDEO) == 0) {
		return true;
	} else {
		LOGF(LogSDL, Error, "SDL_Init Error: %i", SDL_GetError());
		return false;
	}
}

bool SDLFrameworkSystem::Shutdown(CTX_ARG) {
	SDL_Quit();
	return true;
}

bool SDLEventSystem::Startup(CTX_ARG) {
	frameEvents.reserve(20);
	return true;
}

bool SDLEventSystem::Shutdown(CTX_ARG) { return true; }

void SDLEventSystem::PollEvents(bool& requestShutdown) {
	frameEvents.clear();
	SDL_Event currentEvent;

	while (SDL_PollEvent(&currentEvent)) {
		ImGui_ImplSDL2_ProcessEvent(&currentEvent);
		frameEvents.push_back(currentEvent);
		if (currentEvent.type == SDL_QUIT) {
			requestShutdown = true;
		}
	}
}

bool SDLWindowSystem::Startup(CTX_ARG) {
	mainWindow = SDL_CreateWindow("AndoSystem", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
	if (!mainWindow) {
		LOGF(LogSDL, Error, "Failed to create SDL window: %i", SDL_GetError());
		return false;
	}
	return true;
}

bool SDLWindowSystem::Shutdown(CTX_ARG) {
	if (mainWindow) SDL_DestroyWindow(mainWindow);
	return true;
}
