#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "Engine/Context.h"

struct SDLFrameworkSystem {
public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);
};

struct SDLEventSystem {
protected:
	std::vector<SDL_Event> frameEvents;

public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);

	void PollEvents(bool& requestShutdown);
};

struct SDLWindowSystem {
protected:
	SDL_Window* mainWindow;

public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);

	inline SDL_Window* GetMainWindow() const { return mainWindow; }
};
