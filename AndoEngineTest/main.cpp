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
#include "ShaderProgramComponent.h"
#include "ShaderComponent.h"

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

	TCompManager<C::Shader> ShaderManager{};
	TCompInfo<C::Shader> Shader{ 120, "Shader", &ShaderManager };

	TCompManager<C::ShaderProgram> ShaderProgramManager{};
	TCompInfo<C::ShaderProgram> ShaderProgram{ 130, "ShaderProgram", &ShaderProgramManager };

	S::EntitySystem EntitySystem{
		{
			&Transform,
			&Hierarchy,
			&Mesh,
			&MeshRenderer,
			&Shader,
			&ShaderProgram,
		}
	};

	S::SDLSystem SDLSystem;
	S::SDLEventSystem SDLEventSystem;
	S::SDLWindowSystem SDLWindowSystem;

	S::RenderingSystem RenderingSystem{ &EntitySystem, &Mesh, &MeshRenderer, &Shader, &ShaderProgram };

	cout << "Initializing all systems" << endl;
	bool bInitializeSuccessful =
		EntitySystem.Initialize() &&
		SDLSystem.Initialize() &&
		SDLEventSystem.Initialize() &&
		SDLWindowSystem.Initialize();

	if( bInitializeSuccessful )
	{
		cout << "Creating entities" << endl;
		EntityID EntA = 3;
		EntityID EntB = 5;
		EntityID VertexShaderEnt = 40;
		EntityID FragmentShaderEnt = 45;
		EntityID ShaderProgramEnt = 50;
		EntityID MeshEnt = 55;

		EntitySystem.Create( EntA );
		EntitySystem.Create( EntB, { &Transform, &Hierarchy } );

		EntitySystem.Create( VertexShaderEnt, { &Shader } );
		EntitySystem.Create( FragmentShaderEnt, { &Shader } );
		EntitySystem.Create( ShaderProgramEnt, { &ShaderProgram } );

		EntitySystem.Create( MeshEnt, { &Transform, &Mesh, &MeshRenderer } );

		RenderingSystem.SetMesh( MeshEnt, MeshEnt );
		RenderingSystem.SetShaderProgram( MeshEnt, ShaderProgramEnt );
		RenderingSystem.SetShaderSource(
			VertexShaderEnt,
			"#version 330 core\n\
			layout(location = 0) in vec3 vertexPosition_modelspace;\n\
			void main(void) { gl_Position.xyz = vertexPosition_modelspace; gl_Position.w = 1.0; }",
			GL_VERTEX_SHADER
		);
		RenderingSystem.SetShaderSource(
			FragmentShaderEnt,
			"#version 330 core\n\
			out vec3 color;\n\
			void main(){ color = vec3(1,0,0); }",
			GL_FRAGMENT_SHADER
		);

		RenderingSystem.SetProgramShaders( ShaderProgramEnt, { VertexShaderEnt, FragmentShaderEnt } );
		RenderingSystem.Link( ShaderProgramEnt );

		cout << "Current entity system status:" << endl;
		cout << EntitySystem;

		bool bShutdownRequested = false;
		do{
			SDLEventSystem.Update( bShutdownRequested );
			if( !bShutdownRequested )
			{
				SDLWindowSystem.Clear();

				RenderingSystem.Update();

				SDLWindowSystem.Swap();
			}
		}
		while( !bShutdownRequested );
	}

	EntitySystem.Deinitialize();
	SDLWindowSystem.Deinitialize();
	SDLEventSystem.Deinitialize();
	SDLSystem.Deinitialize();

	return 0;
}
