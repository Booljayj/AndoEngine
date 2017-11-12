//
//  main.cpp
//  AndoEngineTest
//
//  Created by Justin Bool on 7/8/17.
//
//

#include <iostream>
using namespace std;

#include <GL/glew.h>

#include "EntityFramework/EntitySystem.h"
#include "Rendering/SDLSystems.h"
#include "Rendering/RenderingSystem.h"

#include "AndoEngine/BasicComponents.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"

#include "Rendering/GLVertexData.h"
#include "Rendering/GLShader.h"

int main( int argc, const char * argv[] )
{
	cout << "Hello, World! This is AndoEngine." << endl;

	TComponentManager<C::TransformComponent> TransformManager{};
	TComponentInfo<C::TransformComponent> Transform{ 1, "Transform", &TransformManager };

	TComponentManager<C::HierarchyComponent> HierarchyManager{};
	TComponentInfo<C::HierarchyComponent> Hierarchy{ 2, "Hierarchy", &HierarchyManager };

	TComponentManager<C::Mesh> MeshManager{};
	TComponentInfo<C::Mesh> Mesh{ 100, "Mesh", &MeshManager };

	TComponentManager<C::MeshRenderer> MeshRendererManager{};
	TComponentInfo<C::MeshRenderer> MeshRenderer{ 110, "MeshRenderer", &MeshRendererManager };

	TComponentManager<C::ShaderComponent> ShaderManager{};
	TComponentInfo<C::ShaderComponent> Shader{ 120, "Shader", &ShaderManager };

	TComponentManager<C::ProgramComponent> ProgramManager{};
	TComponentInfo<C::ProgramComponent> Program{ 130, "Program", &ProgramManager };

	S::EntitySystem EntitySys{
		{
			&Transform,
			&Hierarchy,
			&Mesh,
			&MeshRenderer,
			&Shader,
			&Program,
		}
	};

	S::SDLSystem SDL;
	S::SDLEventSystem SDLEvent;
	S::SDLWindowSystem SDLWindow;

	S::RenderingSystem Rendering{ &EntitySys, &Mesh, &MeshRenderer, &Shader, &Program };

	cout << "Initializing all systems" << endl;
	bool bInitializeSuccessful =
		EntitySys.Initialize() &&
		SDL.Initialize() &&
		SDLEvent.Initialize() &&
		SDLWindow.Initialize();

	if( bInitializeSuccessful )
	{
		cout << "Creating entities" << endl;
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

	EntitySys.Deinitialize();
	SDLWindow.Deinitialize();
	SDLEvent.Deinitialize();
	SDL.Deinitialize();

	return 0;
}
