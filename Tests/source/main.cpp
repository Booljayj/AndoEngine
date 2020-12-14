#include "Engine/Time.h"
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Engine/Logging/TerminalLoggerModule.h"
#include "Engine/Logging/FileLoggerModule.h"
#include "EntityFramework/EntityRegistry.h"
#include "EntityFramework/UtilityMacros.h"
#include "HAL/FrameworkSystem.h"
#include "HAL/EventsSystem.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/RenderingSystem.h"
#include "Profiling/ProfilerMacros.h"

#include "Rendering/MaterialComponent.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"

EntityRegistry registry;

HAL::FrameworkSystem framework;
HAL::EventsSystem events;
HAL::WindowingSystem windowing;

Rendering::RenderingSystem rendering;

DEFINE_LOG_CATEGORY(Main, Debug);
DEFINE_PROFILE_CATEGORY(Main);

// Primary system procedures
bool Startup(CTX_ARG) {
	PROFILE_FUNCTION(Main);
	TEMP_ALLOCATOR_MARK();
	LOG(Main, Info, "Starting up all systems...");

	STARTUP_SYSTEM(Main, framework);
	STARTUP_SYSTEM(Main, events);
	STARTUP_SYSTEM(Main, windowing);
	STARTUP_SYSTEM(Main, rendering, windowing, registry);
	return true;
}

void Shutdown(CTX_ARG) {
	PROFILE_FUNCTION(Main);
	TEMP_ALLOCATOR_MARK();
	LOG(Main, Info, "Shutting down all systems...");

	SHUTDOWN_SYSTEM(Main, rendering, registry);
	SHUTDOWN_SYSTEM(Main, windowing);
	SHUTDOWN_SYSTEM(Main, events);
	SHUTDOWN_SYSTEM(Main, framework);
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

			events.PollEvents(shutdownRequested);

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
		using namespace Rendering;

		EntityHandle testMaterialEntity = registry.Create();
		MaterialComponent& material = testMaterialEntity.Add<MaterialComponent>();
		material.fragment = "default.frag";
		material.vertex = "default.vert";

		EntityHandle testMeshEntity = registry.Create();
		MeshComponent& mesh = testMeshEntity.Add<MeshComponent>();
		mesh.vertices = {
			Vertex_Simple{{-0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}, {0,0,1}, {0,0}},
			Vertex_Simple{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}, {0,0,1}, {0,0}},
			Vertex_Simple{{0.5f, 0.5f, 0.0f}, {0, 0, 255, 255}, {0,0,1}, {0,0}},
			Vertex_Simple{{-0.5f, 0.5f, 0.0f}, {255, 255, 255, 255}, {0,0,1}, {0,0}},
		};
		mesh.indices = {0, 1, 2, 2, 3, 0};

		EntityHandle testMeshRendererEntity = registry.Create();
		MeshRendererComponent& renderer = testMeshRendererEntity.Add<MeshRendererComponent>();
		renderer.material = testMaterialEntity.ID();
		renderer.mesh = testMeshEntity.ID();

		MainLoop(CTX);
	}
	Shutdown(CTX);

	LOGF(Main, Info, "TempBuffer:{ Capacity: %i, Current: %i, Peak: %i }", capacity, CTX.temp.GetUsed(), CTX.temp.GetPeakUsage());
	return 0;
}
