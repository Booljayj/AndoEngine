#pragma once
#include "Engine/Context.h"
#include "HAL/SDL2.h"

namespace HAL {
#if SDL_ENABLED
	using Window = SDL_Window;
#else
	using Window = void;
#endif

	struct WindowingSystem {
	protected:
		Window* mainWindow;

	public:
		bool Startup(CTX_ARG);
		bool Shutdown(CTX_ARG);

		inline Window* GetMainWindow() const { return mainWindow; }
	};
}
