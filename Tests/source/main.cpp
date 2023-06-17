#include "PCH.h"
#include "Engine/Time.h"
#include "Engine/Logging.h"
#include "HAL/EventsSystem.h"
#include "HAL/FrameworkSystem.h"
#include "HAL/SDL2.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/RenderingSystem.h"
#include "Profiling/ProfilerMacros.h"

#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/Shader.h"
#include "Rendering/StaticMesh.h"

#include "ThirdParty/EnTT.h"

#include "Importers/RenderingImporters.h"

#define STARTUP_SYSTEM(Category, System, ...)\
LOG(Category, Info, "Startup "#System);\
if (!System.Startup(__VA_ARGS__)) { LOG(Category, Error, "Failed to startup " #System); return false; }

#define SHUTDOWN_SYSTEM(Category, System, ...)\
LOG(Category, Info, "Shutdown "#System);\
if (!System.Shutdown(__VA_ARGS__)) { LOG(Category, Error, "Failed to shutdown " #System); }

LOG_CATEGORY(Main, Debug);
DEFINE_PROFILE_CATEGORY(Main);

struct Application {
	Resources::Database<
		Rendering::StaticMesh,
		Rendering::Material,
		Rendering::VertexShader,
		Rendering::FragmentShader
	> database;

	entt::registry registry;

	HAL::FrameworkSystem framework;
	HAL::EventsSystem events;
	HAL::WindowingSystem windowing;
	Rendering::RenderingSystem rendering;

	Application() = default;

	// Primary system procedures
	bool Startup() {
		PROFILE_FUNCTION(Main);
		ScopedThreadBufferMark mark;
		LOG(Main, Info, "Starting up all systems...");

		STARTUP_SYSTEM(Main, framework);
		STARTUP_SYSTEM(Main, events);
		STARTUP_SYSTEM(Main, windowing);
		STARTUP_SYSTEM(Main, rendering, windowing, database.GetCache<Rendering::Material>(), database.GetCache<Rendering::StaticMesh>());
		return true;
	}

	void Shutdown() {
		PROFILE_FUNCTION(Main);
		ScopedThreadBufferMark mark;
		LOG(Main, Info, "Shutting down all systems...");

		SHUTDOWN_SYSTEM(Main, rendering, database.GetCache<Rendering::Material>(), database.GetCache<Rendering::StaticMesh>());
		SHUTDOWN_SYSTEM(Main, windowing);
		SHUTDOWN_SYSTEM(Main, events);
		SHUTDOWN_SYSTEM(Main, framework);
	}

	void MainLoop() {
		TimeController_FixedUpdateVariableRendering timeController{60.0f, 10.0f};

		HAL::SystemEvents ev;
		while (!ev.quit) {
			PROFILE_DURATION("MainLoop", Main);
			ScopedThreadBufferMark mark;

			timeController.NextFrame();

			while (timeController.StartUpdate()) {
				//Main Update. Anything inside this loop runs with a fixed interval (possibly simulated based on variable rates)
				//const Time& time = timeController.GetTime();

				events.PollEvents(ev);

				timeController.FinishUpdate();
			}

			if (!ev.quit) {
				//Render. Anything inside this loop runs with a variable interval. Alpha will indicate the progress from the previous to the current main update.
				//const float alpha = timeController.Alpha();
				ev.quit |= !rendering.Render(registry);
			}
		}
	}
};

int main(int argc, char** argv) {
	using namespace Rendering;
	using namespace Resources;

	ThreadBuffer buffer{ 20'000 };
	
	Logger::Get().CreateDevice<TerminalOutputDevice>();

	LOG(Main, Info, "Hello, World! This is AndoEngine.");
	LOG(Main, Debug, "Compiled with " COMPILER_VERSION " on " __DATE__);
	LOGF(Main, Debug, "CWD: %s", std::filesystem::current_path().generic_string().data());

	Application application;

	if (application.Startup()) {
		//Create default built-in vertex and fragment shaders
		Handle<VertexShader> const vertex = application.database.GetCache<VertexShader>().Create(0);
		Handle<FragmentShader> const fragment = application.database.GetCache<FragmentShader>().Create(1);
		{
			Importers::ShaderImporter shaderImporter;
			shaderImporter.Import(
				*vertex,
				R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "uniforms_standard.incl"
#include "attributes_simple.vert.incl"

void main() {
	gl_Position = object.modelViewProjection * vec4(inPosition, 1.0);
	outFragColor = inColor;
}
				)",
				"default.vert"
			);

			shaderImporter.Import(
				*fragment,
				R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "uniforms_standard.incl"
#include "fragments_simple.frag.incl"

void main() {
    outColor = inFragColor;
}
				)",
				"default.frag"
			);
		}

		Handle<Material> const material = application.database.GetCache<Material>().Create(10);
		material->vertex = vertex;
		material->fragment = fragment;

		Handle<StaticMesh> const plane = application.database.GetCache<StaticMesh>().Create(100);
		Vertices_Simple& vertices = plane->vertices.emplace<Vertices_Simple>();
		vertices = {
			{{-0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}, {0,0,1}, {0,0}},
			{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}, {0,0,1}, {0,0}},
			{{0.5f, 0.5f, 0.0f}, {0, 0, 255, 255}, {0,0,1}, {0,0}},
			{{-0.5f, 0.5f, 0.0f}, {255, 255, 255, 255}, {0,0,1}, {0,0}},
		};
		Indices_Short& indices = plane->indices.emplace<Indices_Short>();
		indices = { 0, 1, 2, 2, 3, 0 };

		entt::entity const testEntity = application.registry.create();
		MeshRenderer& renderer = application.registry.emplace<MeshRenderer>(testEntity);
		renderer.material = material;
		renderer.mesh = plane;

		application.MainLoop();
	}
	application.Shutdown();

	buffer.LogDebugStats();

	return 0;
}
