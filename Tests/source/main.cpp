#include <iostream>
#include <GL/glew.h>
#include "Engine/BasicComponents.h"
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/ScopedTempBlock.h"
#include "Engine/Utility.h"
#include "Engine/Print.h"
#include "EntityFramework/EntitySystem.h"
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

CREATE_COMPONENT( 100, Mesh, C::Mesh, C::MeshComponentManager{} );
CREATE_COMPONENT( 110, MeshRenderer, C::MeshRenderer, C::MeshRendererComponentManager{} );
CREATE_COMPONENT( 120, Shader, C::ShaderComponent, C::ShaderComponentManager{} );
CREATE_COMPONENT( 130, Program, C::ProgramComponent, C::ProgramComponentManager{} );

// Systems

S::EntitySystem EntitySys;

S::SDLSystem SDL;
S::SDLEventSystem SDLEvent;
S::SDLWindowSystem SDLWindow;

S::RenderingSystem Rendering{ &MeshRendererManager };

bool Startup( CTX_ARG )
{
	TEMP_SCOPE;
	CTX.Log->Message( "Starting up all systems..." );

	const l_vector<const ComponentInfo*> Components{
		{
			&Transform,
			&Mesh,
			&MeshRenderer,
			&Hierarchy, //not sorted because of this guy
			&Shader,
			&Program,
		},
		CTX.Temp
	};

	STARTUP_SYSTEM_ARGS( EntitySys, Components );
	STARTUP_SYSTEM( SDL );
	STARTUP_SYSTEM( SDLEvent );
	STARTUP_SYSTEM( SDLWindow );
	return true;
}

void Shutdown( CTX_ARG )
{
	TEMP_SCOPE;
	CTX.Log->Message( "Shutting down all systems..." );

	SHUTDOWN_SYSTEM( EntitySys );
	SHUTDOWN_SYSTEM( SDLWindow );
	SHUTDOWN_SYSTEM( SDLEvent );
	SHUTDOWN_SYSTEM( SDL );
}

void MainLoop( CTX_ARG )
{
	double TotalTime = 0.0;
	double DeltaTime = 1.0/60.0;
	auto LastUpdateTime = std::chrono::system_clock::now();
	double AccumulatedTime = 0.0;

	bool bShutdownRequested = false;
	while( !bShutdownRequested ) {

		auto CurrentUpdateTime = std::chrono::system_clock::now();
		double ElapsedTime = ( CurrentUpdateTime - LastUpdateTime ).count();
		ElapsedTime = std::min( ElapsedTime, 0.25 );
		AccumulatedTime += ElapsedTime;

		while( AccumulatedTime >= DeltaTime ) {
			CTX.Temp.Reset();

			SDLEvent.Update( bShutdownRequested );

			TotalTime += DeltaTime;
			AccumulatedTime -= DeltaTime;
		}

		if( !bShutdownRequested )
		{
			//@todo Use this alpha. It should be passed into certain rendering functions to allow them to blend between previous and current states
			//const double AlphaFrameTime = AccumulatedTime / DeltaTime;
			CTX.Temp.Reset();

			SDLWindow.Clear();
			Rendering.Update();
			SDLWindow.Swap();
		}

		LastUpdateTime = CurrentUpdateTime;
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
		EntityID EntB = 5;
		EntityID VertexShaderEnt = 40;
		EntityID FragmentShaderEnt = 45;
		EntityID ShaderProgramEnt = 50;
		EntityID MeshEnt = 55;

		EntitySys.Create( EntA );
		EntitySys.Create( EntB, { &Transform, &Hierarchy } );

		EntitySys.Create( VertexShaderEnt, { &Shader } );
		EntitySys.Create( FragmentShaderEnt, { &Shader } );
		EntitySys.Create( ShaderProgramEnt, { &Program } );

		EntitySys.Create( MeshEnt, { &Transform, &Mesh, &MeshRenderer } );

		C::Mesh* TestMesh = EntitySys.Find( MeshEnt )->Get( Mesh );
		TestMesh->Vertices =
		{
			GL::VertexData{ -1, -1, 0 },
			GL::VertexData{ 1, -1, 0 },
			GL::VertexData{ 0, 1, 0 },
		};
		TestMesh->CreateBuffers();

		C::MeshRenderer* TestMeshRenderer = EntitySys.Find( MeshEnt )->Get( MeshRenderer );
		TestMeshRenderer->Setup( TestMesh );

		C::ShaderComponent* TestVertexShader = EntitySys.Find( VertexShaderEnt )->Get( Shader );
		TestVertexShader->Source =
			"#version 330 core\n\
			in vec3 vert_Position;\n\
			void main(void) { gl_Position.xyz = vert_Position; gl_Position.w = 1.0; }";
		TestVertexShader->ShaderType = GL::EShader::Vertex;

		C::ShaderComponent* TestFragmentShader = EntitySys.Find( FragmentShaderEnt )->Get( Shader );
		TestFragmentShader->Source =
			"#version 330 core\n\
			out vec4 color;\n\
			void main(){ color = vec4(1,0,0,1); }";
		TestFragmentShader->ShaderType = GL::EShader::Fragment;

		C::ProgramComponent* TestProgram = EntitySys.Find( ShaderProgramEnt )->Get( Program );
		TestProgram->LinkedShaders = { TestVertexShader, TestFragmentShader };
		GL::Link( *TestProgram );
		GL::Use( *TestProgram );

		CTX.Log->Message( DESC( EntitySys ) );
		CTX.Log->Message( "Component Descriptions:" );
		for( const ComponentInfo* Info : EntitySys.GetRegisteredComponents() )
		{
			CTX.Log->Message( l_printf( CTX.Temp, "\t%s", DESC( *Info ) ) );
		}

		MainLoop( CTX );
	}

	Shutdown( CTX );
	CTX.Log->Message( DESC( CTX.Temp ) );
	return 0;
}
