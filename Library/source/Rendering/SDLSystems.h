#pragma once
#include <vector>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "Engine/LinearStrings.h"
#include "Engine/Context.h"

namespace S
{
	class SDLSystem
	{
	public:
		bool Startup( CTX_ARG )
		{
			if( SDL_Init( SDL_INIT_VIDEO ) == 0 )
			{
				return true;
			}
			else
			{
				CTX.Log->Error( l_printf( CTX.Temp, "SDL_Init Error: %i", SDL_GetError() ) );
				return false;
			}
		}

		bool Shutdown( CTX_ARG )
		{
			SDL_Quit();
			return true;
		}
	};

	class SDLEventSystem
	{
	public:
		std::vector<SDL_Event> FrameEvents;

		bool Startup( CTX_ARG )
		{
			FrameEvents.reserve( 20 );
			return true;
		}

		bool Shutdown( CTX_ARG ) { return true; }

		void Update( bool& bRequestShutdown )
		{
			FrameEvents.clear();
			SDL_Event CurrentEvent;

			while( SDL_PollEvent( &CurrentEvent ) )
			{
				FrameEvents.push_back( CurrentEvent );
				if( CurrentEvent.type == SDL_QUIT )
				{
					bRequestShutdown = true;
				}
			}
		}
	};

	class SDLWindowSystem
	{
	protected:
		SDL_Window* MainWindow;
		SDL_GLContext MainContext;

	public:
		bool Startup( CTX_ARG )
		{
			MainWindow = SDL_CreateWindow( "AndoSystem", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
			if( MainWindow )
			{
				SetupGLAttributes();
				MainContext = SDL_GL_CreateContext( MainWindow );
				if( MainContext )
				{
					SDL_GL_MakeCurrent( MainWindow, MainContext );
					SDL_GL_SetSwapInterval( 1 );

					glewExperimental = true; // Needed in core profile

					if( glewInit() == GLEW_OK )
					{
						glClearColor(0.0, 0.5, 0.5, 1.0);
						Clear();
						Swap();
						return true;
					}
					CTX.Log->Error( "Failed to initialize GLEW" );

					SDL_GL_DeleteContext( MainContext );
					MainContext = nullptr;
				}
				CTX.Log->Error( "Failed to create OpenGL Context" );

				SDL_DestroyWindow( MainWindow );
				MainWindow = nullptr;
			}
			CTX.Log->Error( l_printf( CTX.Temp, "SDL_Window Error: %i", SDL_GetError() ) );
			return false;
		}

		bool Shutdown( CTX_ARG )
		{
			SDL_GL_DeleteContext( MainContext );
			SDL_DestroyWindow( MainWindow );
			return true;
		}

		void Clear()
		{
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		}

		void Swap()
		{
			SDL_GL_SwapWindow( MainWindow );
		}

	protected:
		void SetupGLAttributes()
		{
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
			//SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
			//SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
			//SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		}
	};
}
