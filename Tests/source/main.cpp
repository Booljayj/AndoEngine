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

#include "Reflection/StandardResolvers.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/TupleTypeInfo.h"
#include "Reflection/ReflectionTest.h"
#include "Reflection/TypeUtility.h"

// Components and managers

CREATE_COMPONENT(   1, Transform, TransformComponent, TransformComponentManager{} );
CREATE_COMPONENT(   2, Hierarchy, HierarchyComponent, HierarchyComponentManager{} );

CREATE_COMPONENT( 100, Mesh, MeshComponent, MeshComponentManager{} );
CREATE_COMPONENT( 110, MeshRenderer, MeshRendererComponent, MeshRendererComponentManager{} );
CREATE_COMPONENT( 120, Shader, ShaderComponent, ShaderComponentManager{} );
CREATE_COMPONENT( 130, Program, ProgramComponent, ProgramComponentManager{} );

// Systems

ComponentCollectionSystem ComponentCollection;
EntityCollectionSystem EntityCollection;

SDLFrameworkSystem SDLFramework;
SDLEventSystem SDLEvent;
SDLWindowSystem SDLWindow;

RenderingSystem Rendering;

DEFINE_LOG_CATEGORY( Main, Debug );

bool Startup( CTX_ARG ) {
	TEMP_SCOPE;
	LOG( LogMain, Info, "Starting up all systems..." );

	const std::initializer_list<const ComponentInfo*> Components = {
		&Transform,
		&Hierarchy,
		&Mesh,
		&MeshRenderer,
		&Shader,
		&Program,
	};

	STARTUP_SYSTEM( LogMain, ComponentCollection, Components.begin(), Components.size() );
	STARTUP_SYSTEM( LogMain, EntityCollection, &ComponentCollection, 100 );
	STARTUP_SYSTEM( LogMain, SDLFramework );
	STARTUP_SYSTEM( LogMain, SDLEvent );
	STARTUP_SYSTEM( LogMain, SDLWindow );
	STARTUP_SYSTEM( LogMain, Rendering, &EntityCollection, &Transform, &MeshRenderer );
	return true;
}

void Shutdown( CTX_ARG ) {
	TEMP_SCOPE;
	LOG( LogMain, Info, "Shutting down all systems..." );

	SHUTDOWN_SYSTEM( LogMain, Rendering );
	SHUTDOWN_SYSTEM( LogMain, SDLWindow );
	SHUTDOWN_SYSTEM( LogMain, SDLEvent );
	SHUTDOWN_SYSTEM( LogMain, SDLFramework );
	SHUTDOWN_SYSTEM( LogMain, EntityCollection );
	SHUTDOWN_SYSTEM( LogMain, ComponentCollection );
}

void MainLoop( CTX_ARG ) {
	TimeController_FixedUpdateVariableRendering TimeController{ 60.0f, 10.0f };

	bool bShutdownRequested = false;
	while( !bShutdownRequested ) {
		TimeController.AdvanceFrame();

		SDLEvent.PollEvents( bShutdownRequested );

		while( TimeController.StartUpdateFrame() ) {
			CTX.Temp.Reset();
			//const Time& T = TimeController.GetTime();
			//CTX.Log->Message( DESC( TimeController ) );
			EntityCollection.UpdateFilters();

			//Game Update

			EntityCollection.RecycleGarbage( CTX );
			TimeController.FinishUpdateFrame();
		}

		if( !bShutdownRequested ) {
			const float InterpolationAlpha = TimeController.FrameInterpolationAlpha();
			CTX.Temp.Reset();

			SDLWindow.Clear();
			Rendering.RenderFrame( InterpolationAlpha );
			SDLWindow.Swap();
		}
	}
}

int main( int argc, char const* argv[] ) {
	Context CTX{ 10000 };
	CTX.Log.AddModule( std::make_shared<TerminalLoggerModule>() );
	//CTX.Log.AddModule( std::make_shared<FileLoggerModule>( "Main.log" ) );

	LOG( LogMain, Info, "Hello, World! This is AndoEngine." );
	LOG( LogMain, Debug, "Compiled with " __VERSION__ " on " __DATE__ );

	if( Startup( CTX ) ) {
		LOG( LogMain, Info, "Creating entities" );
		EntityID EntA = 3;
		EntityID VertexShaderID = 40;
		EntityID FragmentShaderID = 45;
		EntityID ShaderProgramID = 50;
		EntityID MeshID = 55;

		EntityCollection.Create( CTX, EntA, { &Transform, &Hierarchy } );

		Entity const* VertexShaderEntity = EntityCollection.Create( CTX, VertexShaderID, { &Shader } );
		Entity const* FragmentShaderEntity = EntityCollection.Create( CTX, FragmentShaderID, { &Shader } );
		Entity const* ShaderProgramEntity = EntityCollection.Create( CTX, ShaderProgramID, { &Program } );

		Entity const* MeshEntity = EntityCollection.Create( CTX, MeshID, { &Transform, &Mesh, &MeshRenderer } );

		MeshComponent* TestMesh = MeshEntity->Get( Mesh );
		TestMesh->Vertices = {
			GL::VertexData{ -1, -1, 0 },
			GL::VertexData{ 1, -1, 0 },
			GL::VertexData{ 0, 1, 0 },
		};
		TestMesh->CreateBuffers();

		MeshRendererComponent* TestMeshRenderer = MeshEntity->Get( MeshRenderer );
		TestMeshRenderer->Setup( TestMesh );

		ShaderComponent* TestVertexShader = VertexShaderEntity->Get( Shader );
		TestVertexShader->Source =
			"#version 330 core\n\
			in vec3 vert_Position;\n\
			void main(void) { gl_Position.xyz = vert_Position; gl_Position.w = 1.0; }";
		TestVertexShader->ShaderType = GL::EShader::Vertex;

		ShaderComponent* TestFragmentShader = FragmentShaderEntity->Get( Shader );
		TestFragmentShader->Source =
			"#version 330 core\n\
			out vec4 color;\n\
			void main(){ color = vec4(1,0,0,1); }";
		TestFragmentShader->ShaderType = GL::EShader::Fragment;

		ProgramComponent* TestProgram = ShaderProgramEntity->Get( Program );
		TestProgram->LinkedShaders = { TestVertexShader, TestFragmentShader };
		GL::Link( *TestProgram );
		GL::Use( *TestProgram );

		// MainLoop( CTX );
	}

	Shutdown( CTX );

	LOGF(
		LogMain, Info, "[TempBuffer]{ Current: %i/%i, Peak: %i/%i }",
		CTX.Temp.GetUsed(), CTX.Temp.GetCapacity(), CTX.Temp.GetPeakUsage(), CTX.Temp.GetCapacity()
	);

	return 0;
}
