#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "Engine/Context.h"

namespace S
{
	class SDLSystem
	{
	public:
		bool Startup( CTX_ARG );
		bool Shutdown( CTX_ARG );
	};

	class SDLEventSystem
	{
	protected:
		std::vector<SDL_Event> FrameEvents;

	public:
		bool Startup( CTX_ARG );
		bool Shutdown( CTX_ARG );

		void Update( bool& bRequestShutdown );
	};

	class SDLWindowSystem
	{
	protected:
		SDL_Window* MainWindow;
		SDL_GLContext MainContext;

	public:
		bool Startup( CTX_ARG );
		bool Shutdown( CTX_ARG );

		void Clear();
		void Swap();

	protected:
		void SetupGLAttributes();
	};
}
