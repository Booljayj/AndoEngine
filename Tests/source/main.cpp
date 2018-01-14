#include <iostream>
#include <GL/glew.h>
#include "Engine/BasicComponents.h"
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/Utility.h"
#include "EntityFramework/EntitySystem.h"
#include "Rendering/SDLSystems.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/VertexData.h"
#include "Rendering/Shader.h"

using namespace std;

// Components and managers

CREATE_STANDARD_COMPONENT( C::TransformComponent, Transform, 1 );
CREATE_STANDARD_COMPONENT( C::HierarchyComponent, Hierarchy, 2 );

CREATE_STANDARD_COMPONENT( C::Mesh, Mesh, 100 );
CREATE_STANDARD_COMPONENT( C::MeshRenderer, MeshRenderer, 110 );
CREATE_STANDARD_COMPONENT( C::ShaderComponent, Shader, 120 );
CREATE_STANDARD_COMPONENT( C::ProgramComponent, Program, 130 );

// Systems

ComponentInfo* Components[] =
{
	&Transform,
	&Mesh,
	&MeshRenderer,
	&Hierarchy, //not sorted because of this guy
	&Shader,
	&Program,
};

S::EntitySystem EntitySys{ Components };

S::SDLSystem SDL;
S::SDLEventSystem SDLEvent;
S::SDLWindowSystem SDLWindow;

S::RenderingSystem Rendering{ &MeshRenderer };

bool Startup( CTX_ARG )
{
	CTX.Log->Message( "Starting up all systems..." );
	STARTUP_SYSTEM( EntitySys );
	STARTUP_SYSTEM( SDL );
	STARTUP_SYSTEM( SDLEvent );
	STARTUP_SYSTEM( SDLWindow );
	return true;
}

void Shutdown( CTX_ARG )
{
	CTX.Log->Message( "Shutting down all systems..." );
	SHUTDOWN_SYSTEM( EntitySys );
	SHUTDOWN_SYSTEM( SDLWindow );
	SHUTDOWN_SYSTEM( SDLEvent );
	SHUTDOWN_SYSTEM( SDL );
}

int main( int argc, const char * argv[] )
{
	Context CTX{ 0, 10000 };
	StandardLogger MainLogger{};
	CTX.Log = &MainLogger;

	CTX.Log->Message( "Hello, World! This is AndoEngine." );

	// cout << "making temp allocator" << endl;
	// LinearAllocatorData Alloc{ 40000 };
	// cout << Alloc << '\n';
	// //l_string broken; //NOPE! You need to supply the allocator, or this won't compile.
	// l_string s{ "Really long string that we don't want to allocate for.", Alloc };
	// s += " I mean, really long.";
	// s += " Like, ridiculously long in a feeble attempt to get more ";
	// s += "allocations.";
	// cout << s << '\n';
	// cout << Alloc << '\n';

	// l_vector<uint16_t> v{ Alloc };
	// v.push_back( 10 );
	// cout << Alloc << '\n';
	// v.push_back( 100 );
	// cout << Alloc << '\n';
	// v.push_back( 1000 );
	// cout << Alloc << '\n';
	// v.push_back( 10000 );
	// cout << Alloc << '\n';

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

		cout << "Current entity system status:" << endl;
		cout << EntitySys;

		bool bShutdownRequested = false;
		do{
			CTX.Temp.Reset();

			SDLEvent.Update( bShutdownRequested );
			if( !bShutdownRequested )
			{
				SDLWindow.Clear();

				Rendering.Update();

				SDLWindow.Swap();
			}
		}
		while( !bShutdownRequested );
	}

	Shutdown( CTX );
	return 0;
}
