#include <iostream>
#include <GL/glew.h>
#include "Engine/BasicComponents.h"
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/ScopedTempBlock.h"
#include "Engine/Time.h"
#include "Engine/UtilityMacros.h"
#include "Engine/Print.h"
#include "EntityFramework/ComponentCollectionSystem.h"
#include "EntityFramework/Entity.h"
#include "EntityFramework/EntityFrameworkSystem.h"
#include "EntityFramework/UtilityMacros.h"
#include "Rendering/SDLSystems.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/VertexData.h"
#include "Rendering/Shader.h"

using namespace std;

// Components and managers

CREATE_COMPONENT(   1, Transform, C::TransformComponent, C::TransformComponentManager{} );
CREATE_COMPONENT(   2, Hierarchy, C::HierarchyComponent, C::HierarchyComponentManager{} );

CREATE_COMPONENT( 100, Mesh, C::MeshComponent, C::MeshComponentManager{} );
CREATE_COMPONENT( 110, MeshRenderer, C::MeshRendererComponent, C::MeshRendererComponentManager{} );
CREATE_COMPONENT( 120, Shader, C::ShaderComponent, C::ShaderComponentManager{} );
CREATE_COMPONENT( 130, Program, C::ProgramComponent, C::ProgramComponentManager{} );

// Systems

ComponentCollectionSystem ComponentCollection;
S::EntityCollectionSystem EntityCollection;

S::SDLFrameworkSystem SDLFramework;
S::SDLEventSystem SDLEvent;
S::SDLWindowSystem SDLWindow;

S::RenderingSystem Rendering;

bool Startup( CTX_ARG )
{
	TEMP_SCOPE;
	CTX.Log->Message( "Starting up all systems..." );

	const initializer_list<const ComponentInfo*> Components =
	{
		&Transform,
		&Hierarchy,
		&Mesh,
		&MeshRenderer,
		&Shader,
		&Program,
	};

	STARTUP_SYSTEM( ComponentCollection, Components.begin(), Components.size() );
	STARTUP_SYSTEM( EntityCollection, &ComponentCollection, 100 );
	STARTUP_SYSTEM( SDLFramework );
	STARTUP_SYSTEM( SDLEvent );
	STARTUP_SYSTEM( SDLWindow );
	STARTUP_SYSTEM( Rendering, &MeshRendererManager );
	return true;
}

void Shutdown( CTX_ARG )
{
	TEMP_SCOPE;
	CTX.Log->Message( "Shutting down all systems..." );

	SHUTDOWN_SYSTEM( Rendering );
	SHUTDOWN_SYSTEM( SDLWindow );
	SHUTDOWN_SYSTEM( SDLEvent );
	SHUTDOWN_SYSTEM( SDLFramework );
	SHUTDOWN_SYSTEM( EntityCollection );
	SHUTDOWN_SYSTEM( ComponentCollection );
}

void MainLoop( CTX_ARG )
{
	TimeController_FixedUpdateVariableRendering TimeController{ 60.0f, 10.0f };

	bool bShutdownRequested = false;
	while( !bShutdownRequested ) {
		TimeController.AdvanceFrame();

		SDLEvent.PollEvents( bShutdownRequested );

		while( TimeController.StartUpdateFrame() ) {
			//const Time& T = TimeController.GetTime();
			CTX.Temp.Reset();
			//CTX.Log->Message( DESC( TimeController ) );

			TimeController.FinishUpdateFrame();
		}

		if( !bShutdownRequested )
		{
			const float InterpolationAlpha = TimeController.FrameInterpolationAlpha();
			CTX.Temp.Reset();

			SDLWindow.Clear();
			Rendering.RenderFrame( InterpolationAlpha );
			SDLWindow.Swap();
		}
	}
}

int main( int argc, const char * argv[] )
{
	StandardLogger MainLogger{};
	Context CTX{ 0, &MainLogger, 10000 };

	CTX.Log->Message( TERM_Cyan "Hello, World! This is AndoEngine." );
	CTX.Log->Message( TERM_Cyan "Compiled with " __VERSION__ "\n" );

	if( Startup( CTX ) )
	{
		CTX.Log->Message( "Creating entities" );
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

		C::MeshComponent* TestMesh = MeshEntity->Get( Mesh );
		TestMesh->Vertices =
		{
			GL::VertexData{ -1, -1, 0 },
			GL::VertexData{ 1, -1, 0 },
			GL::VertexData{ 0, 1, 0 },
		};
		TestMesh->CreateBuffers();

		C::MeshRendererComponent* TestMeshRenderer = MeshEntity->Get( MeshRenderer );
		TestMeshRenderer->Setup( TestMesh );

		C::ShaderComponent* TestVertexShader = VertexShaderEntity->Get( Shader );
		TestVertexShader->Source =
			"#version 330 core\n\
			in vec3 vert_Position;\n\
			void main(void) { gl_Position.xyz = vert_Position; gl_Position.w = 1.0; }";
		TestVertexShader->ShaderType = GL::EShader::Vertex;

		C::ShaderComponent* TestFragmentShader = FragmentShaderEntity->Get( Shader );
		TestFragmentShader->Source =
			"#version 330 core\n\
			out vec4 color;\n\
			void main(){ color = vec4(1,0,0,1); }";
		TestFragmentShader->ShaderType = GL::EShader::Fragment;

		C::ProgramComponent* TestProgram = ShaderProgramEntity->Get( Program );
		TestProgram->LinkedShaders = { TestVertexShader, TestFragmentShader };
		GL::Link( *TestProgram );
		GL::Use( *TestProgram );

		CTX.Log->Message( DESC( EntityCollection ) );
		CTX.Log->Message( DESC( ComponentCollection ) );
		ComponentCollection.DescribeComponents( CTX );

		MainLoop( CTX );
	}

	Shutdown( CTX );
	CTX.Log->Message( DESC( CTX.Temp ) );
	return 0;
}
