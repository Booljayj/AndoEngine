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
	std::vector<SDL_Event> FrameEvents;

public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);

	void PollEvents(bool& bRequestShutdown);
};

struct SDLWindowSystem {
protected:
	SDL_Window* MainWindow;

public:
	bool Startup(CTX_ARG);
	bool Shutdown(CTX_ARG);

	inline SDL_Window* GetMainWindow() const { return MainWindow; }
};
