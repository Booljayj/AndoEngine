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

#include "Resources/MemoryDatabase.h"
#include "Resources/Text.h"

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
	Resources::MemoryDatabase database;
	
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
		STARTUP_SYSTEM(Main, rendering, windowing, database);
		return true;
	}

	void Shutdown() {
		PROFILE_FUNCTION(Main);
		ScopedThreadBufferMark mark;
		LOG(Main, Info, "Shutting down all systems...");

		SHUTDOWN_SYSTEM(Main, rendering, database);
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
	using namespace glm;
	using namespace Rendering;
	using namespace Resources;

	//Allocate a temporary buffer for the main thread
	ThreadBuffer buffer{ 20'000 };
	
	Logger::Get().AddDevices(std::make_shared<TerminalLogDevice>());

	LOG(Main, Info, "Hello, World! This is AndoEngine.");
	LOG(Main, Debug, "Compiled with " COMPILER_VERSION " on " __DATE__);
	LOG(Main, Debug, "CWD: {}", std::filesystem::current_path().generic_string());

	Application application;

	if (application.Startup()) {
		//Create the default plane mesh. This demonstrates the process of assigning raw vertex and index information for a mesh.
		Handle<StaticMesh> const plane = application.database.Create<StaticMesh>(
			"SM_Plane"_sid, Database::GetTemporary(),
			[](StaticMesh& mesh) {
				mesh.vertices.emplace<Vertices_Simple>() = {
					{ vec3{ -0.5f, -0.5f, 0.0f }, Color{ 255, 0, 0, 255 }, vec3{ 0, 0, 1 }, vec2{ 0, 0 } },
					{ vec3{ 0.5f, -0.5f, 0.0f }, Color{ 0, 255, 0, 255 }, vec3{ 0, 0, 1 }, vec2{ 0, 0 } },
					{ vec3{ 0.5f, 0.5f, 0.0f }, Color{ 0, 0, 255, 255 }, vec3{ 0, 0, 1 }, vec2{ 0, 0 } },
					{ vec3{ -0.5f, 0.5f, 0.0f }, Color{ 255, 255, 255, 255 }, vec3{ 0, 0, 1 }, vec2{ 0, 0 } },
				};

				mesh.indices.emplace<Indices_Short>() = { 0, 1, 2, 2, 3, 0 };
			}
		);

		//Create the default material. This demonstrates recursive resource creation that is thread-safe without deadlocks.
		Handle<Material> const material = application.database.Create<Material>(
			"M_Default"_sid, Database::GetTemporary(),
			[&](Material& material) {
				//Shaders are imported from raw text strings. This demonstrates the use of an importer object to initialize resources from raw data.
				Importers::ShaderImporter importer;

				material.shaders.vertex = application.database.Create<VertexShader>(
					"SH_DefaultVertex"_sid, Database::GetTemporary(),
					[&](VertexShader& shader) {
						importer.Import(
							shader,
							R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "uniforms_standard.incl"
#include "attributes_simple.vert.incl"

void main() {
	gl_Position = object.modelViewProjection * vec4(inPosition, 1.0);
	outFragColor = inColor;
}
					)"sv,
							"default.vert"sv
						);
					}
				);

				material.shaders.fragment = application.database.Create<FragmentShader>(
					"SH_DefaultFragment"_sid, Database::GetTemporary(),
					[&](FragmentShader& shader) {
						importer.Import(
							shader,
							R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "uniforms_standard.incl"
#include "fragments_simple.frag.incl"

void main() {
    outColor = inFragColor;
}
					)"sv,
							"default.frag"sv
						);
					}
				);
			}
		);

		//Create a renderer that will show the default plane with the default material. This demonstrates the creation and initialization of entities.
		entt::entity const testEntity = application.registry.create();
		MeshRenderer& renderer = application.registry.emplace<MeshRenderer>(testEntity);
		renderer.material = material;
		renderer.mesh = plane;

		Handle<Text> text = application.database.Create<Text>(
			"T_Test"_sid, Database::GetTemporary(),
			[](Text& text) {
				text.string = "This is a test string. It should have exactly eleven words.";
			}
		);

		Reflection::StructTypeInfo const& type = Reflect<Text>::Get();
		LOG(Temp, Warning, "Name: {}, ID: {}, Vars: {}", type.name, type.id, type.GetVariables().size());

		/*if (Reflection::VariableInfo const* variable = type.GetVariableRange().FindVariable("string"sv))
		{
			LOG(Temp, Warning, "Variable -- Name: {}, ID: {}, Type: {}", variable->name, variable->id, variable->type->name);
			void const* instance = variable->GetImmutable(text.get());

			LOG(Temp, Warning, "Address: {}, Expected: {}", reinterpret_cast<uintptr_t>(instance), reinterpret_cast<uintptr_t>(text.get()) + offsetof(Text, string));
		}*/

		{
			//Serialization test - convert resource objects to YAML to determine if the output is formatted the right way
			YAML::Node shaderNode = Reflect<VertexShader>::Get().Serialize(material->shaders.vertex.get());
			LOG(Temp, Warning, "Shader: {}", YAML::Dump(shaderNode));

			YAML::Node textNode = Reflect<Text>::Get().Serialize(text.get());
			LOG(Temp, Warning, "Text: {}", YAML::Dump(textNode));
		}

		application.MainLoop();
	}
	application.Shutdown();
	const auto temporary = application.database.GetTemporary();

	buffer.LogDebugStats();

	return 0;
}
