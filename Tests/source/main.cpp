#include "PCH.h"
#include "Engine/Time.h"
#include "Engine/Logging.h"
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
#include "Rendering/StaticMeshResource.h"

LOG_CATEGORY(Main, Debug);
DEFINE_PROFILE_CATEGORY(Main);

struct Application {
	EntityRegistry registry;

	HAL::FrameworkSystem framework;
	HAL::EventsSystem events;
	HAL::WindowingSystem windowing;

	Resources::DummyResourceManifest manifest;

	Rendering::RenderingSystem rendering;
	Rendering::StaticMeshResourceDatabase staticMeshResourceDatabase;

	Application()
		: staticMeshResourceDatabase(manifest)
	{}

	// Primary system procedures
	bool Startup() {
		PROFILE_FUNCTION(Main);
		SCOPED_TEMPORARIES();
		LOG(Main, Info, "Starting up all systems...");

		STARTUP_SYSTEM(Main, framework);
		STARTUP_SYSTEM(Main, events);
		STARTUP_SYSTEM(Main, windowing);
		STARTUP_SYSTEM(Main, rendering, windowing, registry);
		return true;
	}

	void Shutdown() {
		PROFILE_FUNCTION(Main);
		SCOPED_TEMPORARIES();
		LOG(Main, Info, "Shutting down all systems...");

		SHUTDOWN_SYSTEM(Main, rendering, registry);
		SHUTDOWN_SYSTEM(Main, windowing);
		SHUTDOWN_SYSTEM(Main, events);
		SHUTDOWN_SYSTEM(Main, framework);
	}

	void MainLoop() {
		TimeController_FixedUpdateVariableRendering timeController{60.0f, 10.0f};

		bool shutdownRequested = false;
		while (!shutdownRequested) {
			PROFILE_DURATION("MainLoop", Main);
			SCOPED_TEMPORARIES();

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
				shutdownRequested |= !rendering.Render(registry);
			}
		}
	}
};

int32_t main(int32_t argc, char const* argv[]) {
	//Create the heap buffer for the main thread
	HeapBuffer buffer{ 20'000 };
	AssignThreadTemporaryBuffer(buffer);

	Logger::Get().CreateDevice<TerminalOutputDevice>();

	LOG(Main, Info, "Hello, World! This is AndoEngine.");
	LOG(Main, Debug, "Compiled with " __VERSION__ " on " __DATE__);

	Application application;

	if (application.Startup()) {
		using namespace Rendering;

		const Resources::Handle<StaticMeshResource> plane = application.staticMeshResourceDatabase.Create(0);
		plane->vertices = FormattedVertices{
			std::in_place_type_t<std::vector<Vertex_Simple>>{},
			{
				Vertex_Simple{{-0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}, {0,0,1}, {0,0}},
				Vertex_Simple{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}, {0,0,1}, {0,0}},
				Vertex_Simple{{0.5f, 0.5f, 0.0f}, {0, 0, 255, 255}, {0,0,1}, {0,0}},
				Vertex_Simple{{-0.5f, 0.5f, 0.0f}, {255, 255, 255, 255}, {0,0,1}, {0,0}},
			}
		};
		plane->indices = FormattedIndices{
			std::in_place_type_t<std::vector<uint16_t>>{},
			std::initializer_list<uint16_t>{0, 1, 2, 2, 3, 0}
		};

		EntityHandle testMaterialEntity = application.registry.Create();
		MaterialComponent& material = testMaterialEntity.Add<MaterialComponent>();
		material.fragment = "default.frag";
		material.vertex = "default.vert";

		EntityHandle testMeshEntity = application.registry.Create();
		MeshComponent& mesh = testMeshEntity.Add<MeshComponent>();
		mesh.vertices = {
			Vertex_Simple{{-0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}, {0,0,1}, {0,0}},
			Vertex_Simple{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}, {0,0,1}, {0,0}},
			Vertex_Simple{{0.5f, 0.5f, 0.0f}, {0, 0, 255, 255}, {0,0,1}, {0,0}},
			Vertex_Simple{{-0.5f, 0.5f, 0.0f}, {255, 255, 255, 255}, {0,0,1}, {0,0}},
		};
		mesh.indices = {0, 1, 2, 2, 3, 0};

		EntityHandle testMeshRendererEntity = application.registry.Create();
		MeshRendererComponent& renderer = testMeshRendererEntity.Add<MeshRendererComponent>();
		renderer.material = testMaterialEntity.ID();
		renderer.mesh = testMeshEntity.ID();

		application.MainLoop();
	}
	application.Shutdown();

	LOGF(Main, Info, "Main Thread HeapBuffer:{ Capacity: %i, Current: %i, Peak: %i }", buffer.GetCapacity(), buffer.GetUsed(), buffer.GetPeakUsage());
	return 0;
}
