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

#include "Rendering/MaterialComponent.h"
#include "Rendering/MeshRendererComponent.h"

EntityRegistry registry;

SDLFrameworkSystem sdlFramework;
SDLEventsSystem sdlEvents;
SDLWindowingSystem sdlWindowing;

Rendering::RenderingSystem rendering;

DEFINE_LOG_CATEGORY(Main, Debug);
DEFINE_PROFILE_CATEGORY(Main);

// Primary system procedures
bool Startup(CTX_ARG) {
	PROFILE_FUNCTION(Main);
	TEMP_ALLOCATOR_MARK();
	LOG(Main, Info, "Starting up all systems...");

	STARTUP_SYSTEM(Main, sdlFramework);
	STARTUP_SYSTEM(Main, sdlEvents);
	STARTUP_SYSTEM(Main, sdlWindowing);
	STARTUP_SYSTEM(Main, rendering, sdlWindowing, registry);
	return true;
}

void Shutdown(CTX_ARG) {
	PROFILE_FUNCTION(Main);
	TEMP_ALLOCATOR_MARK();
	LOG(Main, Info, "Shutting down all systems...");

	SHUTDOWN_SYSTEM(Main, rendering, registry);
	SHUTDOWN_SYSTEM(Main, sdlWindowing);
	SHUTDOWN_SYSTEM(Main, sdlEvents);
	SHUTDOWN_SYSTEM(Main, sdlFramework);
}

void MainLoop(CTX_ARG) {
	TimeController_FixedUpdateVariableRendering timeController{60.0f, 10.0f};

	bool shutdownRequested = false;
	while (!shutdownRequested) {
		PROFILE_DURATION("MainLoop", Main);
		TEMP_ALLOCATOR_MARK();

		timeController.NextFrame();

		while (timeController.StartUpdate()) {
			//Main Update. Anything inside this loop runs with a fixed interval (possibly simulated based on variable rates)
			//const Time& time = timeController.GetTime();

			sdlEvents.PollEvents(shutdownRequested);

			timeController.FinishUpdate();
		}

		if (!shutdownRequested) {
			//Render. Anything inside this loop runs with a variable interval. Alpha will indicate the progress from the previous to the current main update.
			//const float alpha = timeController.Alpha();
			shutdownRequested |= !rendering.Render(CTX, registry);
		}
	}
}

int32_t main(int32_t argc, char const* argv[]) {
	constexpr size_t capacity = 10'000;
	Context CTX{capacity};
	CTX.log.AddModule(std::make_shared<TerminalLoggerModule>());
	//CTX.Log.AddModule(std::make_shared<FileLoggerModule>("Main.log"));

	LOG(Main, Info, "Hello, World! This is AndoEngine.");
	LOG(Main, Debug, "Compiled with " __VERSION__ " on " __DATE__);

	if (Startup(CTX)) {
		EntityHandle material = registry.Create();
		material.Add<Rendering::MaterialComponent>("default.vert", "default.frag");

		EntityHandle meshRenderer = registry.Create();
		meshRenderer.Add<Rendering::MeshRendererComponent>(material.ID());

		MainLoop(CTX);
	}
	Shutdown(CTX);

	LOGF(Main, Info, "TempBuffer:{ Capacity: %i, Current: %i, Peak: %i }", capacity, CTX.temp.GetUsed(), CTX.temp.GetPeakUsage());
	return 0;
}
