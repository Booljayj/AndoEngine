#pragma once
#include <SDL2/SDL.h>
#include "Engine/Context.h"
#include "Engine/STL.h"

struct SDLFrameworkSystem {
public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);
};

struct SDLEventsSystem {
protected:
	std::vector<SDL_Event> frameEvents;

public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);

	void PollEvents(bool& requestShutdown);
};

struct SDLWindowingSystem {
protected:
	SDL_Window* mainWindow;

public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);

	inline SDL_Window* GetMainWindow() const { return mainWindow; }
};
