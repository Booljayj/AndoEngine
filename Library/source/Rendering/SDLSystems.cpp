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
		LOGF(SDL, Error, "SDL_Init Error: %i", SDL_GetError());
		return false;
	}
}

bool SDLFrameworkSystem::Shutdown(CTX_ARG) {
	SDL_Quit();
	return true;
}

bool SDLEventsSystem::Startup(CTX_ARG) {
	frameEvents.reserve(20);
	return true;
}

bool SDLEventsSystem::Shutdown(CTX_ARG) { return true; }

void SDLEventsSystem::PollEvents(bool& requestShutdown) {
	frameEvents.clear();
	SDL_Event currentEvent;

	while (SDL_PollEvent(&currentEvent)) {
		//ImGui_ImplSDL2_ProcessEvent(&currentEvent);
		frameEvents.push_back(currentEvent);

		requestShutdown |= (currentEvent.type == SDL_QUIT);
	}
}

bool SDLWindowingSystem::Startup(CTX_ARG) {
	mainWindow = SDL_CreateWindow("AndoSystem", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
	if (!mainWindow) {
		LOGF(SDL, Error, "Failed to create SDL window: %i", SDL_GetError());
		return false;
	}
	return true;
}

bool SDLWindowingSystem::Shutdown(CTX_ARG) {
	if (mainWindow) SDL_DestroyWindow(mainWindow);
	mainWindow = nullptr;
	return true;
}
