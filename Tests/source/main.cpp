#include <iostream>
#include <string_view>
#include <GL/glew.h>
#include "Engine/BasicComponents.h"
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/ScopedTempBlock.h"
#include "Engine/TerminalColors.h"
#include "Engine/Time.h"
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Engine/Logging/TerminalLoggerModule.h"
#include "Engine/Logging/FileLoggerModule.h"
#include "EntityFramework/ComponentCollectionSystem.h"
#include "EntityFramework/Entity.h"
#include "EntityFramework/EntityCollectionSystem.h"
#include "EntityFramework/UtilityMacros.h"
#include "Rendering/SDLSystems.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/VertexData.h"
#include "Rendering/Shader.h"
#include "Profiling/ProfilerMacros.h"
#include "Reflection/StandardResolvers.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/TupleTypeInfo.h"
#include "Reflection/ReflectionTest.h"
#include "Reflection/TypeUtility.h"

// components and managers
CREATE_COMPONENT(   1, transform, TransformComponent, TransformComponentManager{} );
CREATE_COMPONENT(   2, hierarchy, HierarchyComponent, HierarchyComponentManager{} );

CREATE_COMPONENT( 100, mesh, MeshComponent, MeshComponentManager{} );
CREATE_COMPONENT( 110, meshRenderer, MeshRendererComponent, MeshRendererComponentManager{} );
CREATE_COMPONENT( 120, shader, ShaderComponent, ShaderComponentManager{} );
CREATE_COMPONENT( 130, program, ProgramComponent, ProgramComponentManager{} );

// Systems
ComponentCollectionSystem componentCollectionSystem;
EntityCollectionSystem entityCollectionSystem;

SDLFrameworkSystem sdlFrameworkSystem;
SDLEventSystem sdlEventSystem;
SDLWindowSystem sdlWindowSystem;

RenderingSystem renderingSystem;

DEFINE_LOG_CATEGORY(Main, Debug);

// Primary system procedures
bool Startup(CTX_ARG) {
	PROFILE_FUNCTION();
	TEMP_SCOPE;
	LOG(Main, Info, "Starting up all systems...");

	const std::initializer_list<const ComponentInfo*> components = {
		&transform,
		&hierarchy,
		&mesh,
		&meshRenderer,
		&shader,
		&program,
	};

	STARTUP_SYSTEM(Main, componentCollectionSystem, components.begin(), components.size());
	STARTUP_SYSTEM(Main, entityCollectionSystem, &componentCollectionSystem, 100);
	STARTUP_SYSTEM(Main, sdlFrameworkSystem);
	STARTUP_SYSTEM(Main, sdlEventSystem);
	STARTUP_SYSTEM(Main, sdlWindowSystem);
	STARTUP_SYSTEM(Main, renderingSystem, &sdlWindowSystem, &entityCollectionSystem, &transform, &meshRenderer);
	return true;
}

void Shutdown(CTX_ARG) {
	PROFILE_FUNCTION();
	TEMP_SCOPE;
	LOG(Main, Info, "Shutting down all systems...");

	SHUTDOWN_SYSTEM(Main, renderingSystem);
	SHUTDOWN_SYSTEM(Main, sdlWindowSystem);
	SHUTDOWN_SYSTEM(Main, sdlEventSystem);
	SHUTDOWN_SYSTEM(Main, sdlFrameworkSystem);
	SHUTDOWN_SYSTEM(Main, entityCollectionSystem);
	SHUTDOWN_SYSTEM(Main, componentCollectionSystem);
}

void MainLoop(CTX_ARG) {
	TimeController_FixedUpdateVariableRendering timeController{60.0f, 10.0f};

	bool shutdownRequested = false;
	while (!shutdownRequested) {
		PROFILE_DURATION("MainLoop");

		timeController.AdvanceFrame();

		sdlEventSystem.PollEvents(shutdownRequested);

		while (timeController.StartUpdateFrame()) {
			CTX.temp.Reset();
			//const Time& T = timeController.GetTime();
			entityCollectionSystem.UpdateFilters();

			//Game Update

			entityCollectionSystem.RecycleGarbage(CTX);
			timeController.FinishUpdateFrame();
		}

		if (!shutdownRequested) {
			//const float interpolationAlpha = timeController.FrameInterpolationAlpha();
			CTX.temp.Reset();
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
		// LOG(LogMain, Info, "Creating entities");
		// EntityID entA = 3;
		// EntityID vertexShaderID = 40;
		// EntityID fragmentShaderID = 45;
		// EntityID shaderProgramID = 50;
		// EntityID meshID = 55;

		// entityCollectionSystem.Create(CTX, entA, {&transform, &hierarchy});

		// Entity const* vertexShaderEntity = entityCollectionSystem.Create(CTX, vertexShaderID, {&shader});
		// Entity const* fragmentShaderEntity = entityCollectionSystem.Create(CTX, fragmentShaderID, {&shader});
		// Entity const* shaderProgramEntity = entityCollectionSystem.Create(CTX, shaderProgramID, {&program});

		// Entity const* meshEntity = entityCollectionSystem.Create(CTX, meshID, {&transform, &mesh, &meshRenderer});

		// MeshComponent* testMesh = meshEntity->Get(mesh);
		// testMesh->vertices = {
		// 	GL::VertexData{ -1, -1, 0 },
		// 	GL::VertexData{ 1, -1, 0 },
		// 	GL::VertexData{ 0, 1, 0 },
		// };
		// testMesh->CreateBuffers();

		// MeshRendererComponent* testMeshRenderer = meshEntity->Get(meshRenderer);
		// testMeshRenderer->Setup(testMesh);

		// ShaderComponent* testVertexShader = vertexShaderEntity->Get(shader);
		// testVertexShader->source =
		// 	"#version 330 core\n\
		// 	in vec3 vert_Position;\n\
		// 	void main(void) { gl_Position.xyz = vert_Position; gl_Position.w = 1.0; }";
		// testVertexShader->shaderType = GL::EShader::Vertex;

		// ShaderComponent* testFragmentShader = fragmentShaderEntity->Get( shader );
		// testFragmentShader->source =
		// 	"#version 330 core\n\
		// 	out vec4 color;\n\
		// 	void main(){ color = vec4(1,0,0,1); }";
		// testFragmentShader->shaderType = GL::EShader::Fragment;

		// ProgramComponent* testProgram = shaderProgramEntity->Get(program);
		// testProgram->linkedShaders = {testVertexShader, testFragmentShader};
		// GL::Link(*testProgram);
		// GL::Use(*testProgram);

		// MainLoop(CTX);
	}

	Shutdown(CTX);

	LOGF(
		Main, Info, "[TempBuffer]{ Current: %i/%i, Peak: %i/%i }",
		CTX.temp.GetUsed(), CTX.temp.GetCapacity(), CTX.temp.GetPeakUsage(), CTX.temp.GetCapacity()
	);

	return 0;
}
