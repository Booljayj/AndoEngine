#include "Engine/Time.h"
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Engine/Logging/TerminalLoggerModule.h"
#include "Engine/Logging/FileLoggerModule.h"
#include "EntityFramework/EntityRegistry.h"
#include "EntityFramework/UtilityMacros.h"
#include "Rendering/SDLSystems.h"
#include "Rendering/RenderingSystem.h"
#include "Profiling/ProfilerMacros.h"

EntityRegistry registry;

SDLFrameworkSystem sdlFrameworkSystem;
SDLEventSystem sdlEventSystem;
SDLWindowSystem sdlWindowSystem;

RenderingSystem renderingSystem;

DEFINE_LOG_CATEGORY(Main, Debug);
DEFINE_PROFILE_CATEGORY(Main);

// Primary system procedures
bool Startup(CTX_ARG) {
	PROFILE_FUNCTION(Main);
	TEMP_ALLOCATOR_MARK();
	LOG(Main, Info, "Starting up all systems...");

	STARTUP_SYSTEM(Main, sdlFrameworkSystem);
	STARTUP_SYSTEM(Main, sdlEventSystem);
	STARTUP_SYSTEM(Main, sdlWindowSystem);
	STARTUP_SYSTEM(Main, renderingSystem, sdlWindowSystem, registry);
	return true;
}

void Shutdown(CTX_ARG) {
	PROFILE_FUNCTION(Main);
	TEMP_ALLOCATOR_MARK();

	LOG(Main, Info, "Shutting down all systems...");

	SHUTDOWN_SYSTEM(Main, renderingSystem, registry);
	SHUTDOWN_SYSTEM(Main, sdlWindowSystem);
	SHUTDOWN_SYSTEM(Main, sdlEventSystem);
	SHUTDOWN_SYSTEM(Main, sdlFrameworkSystem);
}

void MainLoop(CTX_ARG) {
	TimeController_FixedUpdateVariableRendering timeController{60.0f, 10.0f};

	bool shutdownRequested = false;
	while (!shutdownRequested) {
		PROFILE_DURATION("MainLoop", Main);
		TEMP_ALLOCATOR_MARK();

		timeController.AdvanceFrame();

		sdlEventSystem.PollEvents(shutdownRequested);

		while (timeController.StartUpdateFrame()) {
			//const Time& time = timeController.GetTime();

			//@todo: Game Update

			timeController.FinishUpdateFrame();
		}

		if (!shutdownRequested) {
			//const float interpolationAlpha = timeController.FrameInterpolationAlpha();
			shutdownRequested |= !renderingSystem.Update(CTX, registry);
		}
	}
}

int main(int argc, char const* argv[]) {
	Context CTX{10000};
	CTX.log.AddModule(std::make_shared<TerminalLoggerModule>());
	//CTX.Log.AddModule( std::make_shared<FileLoggerModule>( "Main.log" ) );

	LOG(Main, Info, "Hello, World! This is AndoEngine.");
	LOG(Main, Debug, "Compiled with " __VERSION__ " on " __DATE__);

	if (Startup(CTX)) {
		MainLoop(CTX);
	}

	Shutdown(CTX);

	LOGF(
		Main, Info, "[TempBuffer]{ Current: %i/%i, Peak: %i/%i }",
		CTX.temp.GetUsed(), CTX.temp.GetCapacity(), CTX.temp.GetPeakUsage(), CTX.temp.GetCapacity()
	);

	return 0;
}
