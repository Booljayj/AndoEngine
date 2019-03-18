#include <GL/glew.h>
#include "Rendering/SDLSystems.h"
#include "Engine/Context.h"
#include "Engine/Logging/LogCategory.h"
#include "Engine/LogCommands.h"
#include "Engine/LinearStrings.h"
#include "UI/IMGUI/imgui.h"
#include "UI/IMGUI/imgui_impl_sdl.h"
#include "UI/IMGUI/imgui_impl_opengl3.h"

DEFINE_LOG_CATEGORY_STATIC( LogSDL, Debug );

bool SDLFrameworkSystem::Startup( CTX_ARG )
{
	if( SDL_Init( SDL_INIT_VIDEO ) == 0 ) {
		return true;
	} else {
		LOGF( LogSDL, Error, "SDL_Init Error: %i", SDL_GetError() );
		return false;
	}
}

bool SDLFrameworkSystem::Shutdown( CTX_ARG )
{
	SDL_Quit();
	return true;
}

bool SDLEventSystem::Startup( CTX_ARG )
{
	FrameEvents.reserve( 20 );
	return true;
}

bool SDLEventSystem::Shutdown( CTX_ARG ) { return true; }

void SDLEventSystem::PollEvents( bool& bRequestShutdown )
{
	FrameEvents.clear();
	SDL_Event CurrentEvent;

	while( SDL_PollEvent( &CurrentEvent ) )
	{
		ImGui_ImplSDL2_ProcessEvent( &CurrentEvent );
		FrameEvents.push_back( CurrentEvent );
		if( CurrentEvent.type == SDL_QUIT )
		{
			bRequestShutdown = true;
		}
	}
}

bool SDLWindowSystem::Startup( CTX_ARG )
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
				ImGui::CreateContext();
				ImGui_ImplSDL2_InitForOpenGL( MainWindow, MainContext );
				ImGui_ImplOpenGL3_Init( "#version 150" );
				ImGui::StyleColorsDark();

				glClearColor(0.0, 0.5, 0.5, 1.0);
				Clear();
				Swap();
				return true;
			}
			LOG( LogSDL, Error, "Failed to initialize GLEW" );

			SDL_GL_DeleteContext( MainContext );
			MainContext = nullptr;
		}
		LOG( LogSDL, Error, "Failed to create OpenGL Context" );

		SDL_DestroyWindow( MainWindow );
		MainWindow = nullptr;
	}
	LOGF( LogSDL, Error, "SDL_Window Error: %i", SDL_GetError() );
	return false;
}

bool SDLWindowSystem::Shutdown( CTX_ARG )
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext( MainContext );
	SDL_DestroyWindow( MainWindow );
	return true;
}

void SDLWindowSystem::Clear()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void SDLWindowSystem::Swap()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame( MainWindow );
	ImGui::NewFrame();

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

	SDL_GL_SwapWindow( MainWindow );
}

void SDLWindowSystem::SetupGLAttributes()
{
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
}
