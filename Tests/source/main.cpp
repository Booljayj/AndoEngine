#include "PCH.h"
#include "Engine/Time.h"
#include "Engine/Logging.h"
#include "EntityFramework/EntityRegistry.h"
#include "EntityFramework/UtilityMacros.h"
#include "HAL/EventsSystem.h"
#include "HAL/FrameworkSystem.h"
#include "HAL/SDL2.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/RenderingSystem.h"
#include "Profiling/ProfilerMacros.h"

#include "Rendering/Materials.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/Shaders.h"
#include "Rendering/StaticMeshResource.h"

#include "Importers/RenderingImporters.h"

LOG_CATEGORY(Main, Debug);
DEFINE_PROFILE_CATEGORY(Main);

struct Application {
	EntityRegistry registry;

	HAL::FrameworkSystem framework;
	HAL::EventsSystem events;
	HAL::WindowingSystem windowing;

	Resources::DummyResourceManifest manifest;

	Rendering::RenderingSystem rendering;
	Rendering::MaterialDatabase materials;
	Rendering::ShaderDatabase shaders;
	Rendering::StaticMeshResourceDatabase staticMeshes;

	Application()
		: materials(manifest)
		, shaders(manifest)
		, staticMeshes(manifest)
	{}

	// Primary system procedures
	bool Startup() {
		PROFILE_FUNCTION(Main);
		SCOPED_TEMPORARIES();
		LOG(Main, Info, "Starting up all systems...");

		STARTUP_SYSTEM(Main, framework);
		STARTUP_SYSTEM(Main, events);
		STARTUP_SYSTEM(Main, windowing);
		STARTUP_SYSTEM(Main, rendering, windowing, registry, materials);
		return true;
	}

	void Shutdown() {
		PROFILE_FUNCTION(Main);
		SCOPED_TEMPORARIES();
		LOG(Main, Info, "Shutting down all systems...");

		SHUTDOWN_SYSTEM(Main, rendering, registry, materials);
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

int main(int argc, char** argv) {
	//Create the heap buffer for the main thread
	HeapBuffer buffer{ 20'000 };
	AssignThreadTemporaryBuffer(buffer);

	Logger::Get().CreateDevice<TerminalOutputDevice>();

	LOG(Main, Info, "Hello, World! This is AndoEngine.");
	LOG(Main, Debug, "Compiled with " COMPILER_VERSION " on " __DATE__);
	LOGF(Main, Debug, "CWD: %s", std::filesystem::current_path().generic_string().data());

	Application application;

	if (application.Startup()) {
		using namespace Rendering;
		using namespace Resources;

		//Create default built-in vertex and fragment shaders
		Handle<VertexShader> const vertex = application.shaders.Create<VertexShader>(0);
		Handle<FragmentShader> const fragment = application.shaders.Create<FragmentShader>(1);
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

		Handle<Material> const material = application.materials.Create(10);
		material->vertex = vertex;
		material->fragment = fragment;

		const Handle<StaticMeshResource> plane = application.staticMeshes.Create(100);
		plane->vertices = FormattedVertices{
			std::in_place_type<std::vector<Vertex_Simple>>,
			{
				Vertex_Simple{{-0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}, {0,0,1}, {0,0}},
				Vertex_Simple{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}, {0,0,1}, {0,0}},
				Vertex_Simple{{0.5f, 0.5f, 0.0f}, {0, 0, 255, 255}, {0,0,1}, {0,0}},
				Vertex_Simple{{-0.5f, 0.5f, 0.0f}, {255, 255, 255, 255}, {0,0,1}, {0,0}},
			}
		};
		plane->indices = FormattedIndices{
			std::in_place_type<std::vector<uint16_t>>,
			std::initializer_list<uint16_t>{0, 1, 2, 2, 3, 0}
		};

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
		renderer.material = material;
		renderer.mesh = testMeshEntity.ID();

		application.MainLoop();
	}
	application.Shutdown();

	LOGF(Main, Info, "Main Thread HeapBuffer:{ Capacity: %i, Current: %i, Peak: %i }", buffer.GetCapacity(), buffer.GetUsed(), buffer.GetPeakUsage());
	return 0;
}
