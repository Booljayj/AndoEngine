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

#include "EntitySystem.h"
#include "SDLSystems.h"
#include "RenderingSystem.h"

#include "BasicComponents.h"
#include "MeshComponent.h"
#include "MeshRendererComponent.h"

#include "GLVertexData.h"
#include "GLShader.h"

int main( int argc, const char * argv[] )
{
	cout << "Hello, World! This is AndoEngine." << endl;

	TCompManager<C::Transform> TransformManager{};
	TCompInfo<C::Transform> Transform{ 1, "Transform", &TransformManager };

	TCompManager<C::Hierarchy> HierarchyManager{};
	TCompInfo<C::Hierarchy> Hierarchy{ 2, "Hierarchy", &HierarchyManager };

	TCompManager<C::Mesh> MeshManager{};
	TCompInfo<C::Mesh> Mesh{ 100, "Mesh", &MeshManager };

	TCompManager<C::MeshRenderer> MeshRendererManager{};
	TCompInfo<C::MeshRenderer> MeshRenderer{ 110, "MeshRenderer", &MeshRendererManager };

	TCompManager<C::ShaderComponent> ShaderManager{};
	TCompInfo<C::ShaderComponent> Shader{ 120, "Shader", &ShaderManager };

	TCompManager<C::ProgramComponent> ProgramManager{};
	TCompInfo<C::ProgramComponent> Program{ 130, "Program", &ProgramManager };

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
