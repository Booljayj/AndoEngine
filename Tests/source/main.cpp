#include <iostream>
#include <string_view>
#include <GL/glew.h>
#include "Engine/StringID.h"
#include "Engine/BasicComponents.h"
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/ScopedTempBlock.h"
#include "Engine/Time.h"
#include "Engine/Context.h"
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

#include "Reflection/Resolver/Resolver.h"
#include "Reflection/StructTypeInfo.h"
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

bool Startup( CTX_ARG )
{
	TEMP_SCOPE;
	CTX.Log->Message( "Starting up all systems..." );

	const std::initializer_list<const ComponentInfo*> Components =
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
	STARTUP_SYSTEM( Rendering, &EntityCollection, &Transform, &MeshRenderer );
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
			CTX.Temp.Reset();
			//const Time& T = TimeController.GetTime();
			//CTX.Log->Message( DESC( TimeController ) );
			EntityCollection.UpdateFilters();

			//Game Update

			EntityCollection.RecycleGarbage( CTX );
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
	CTX.Log->Message( TERM_Cyan "Compiled with " __VERSION__ " on " __DATE__ "\n" );

	std::cout << "\nGlobal types:" << std::endl;
	Reflection::TypeInfo::PrintAll( std::cout );

	std::cout << "Name of std::map<size_t,std::vector<std::array<char,3>>>: " << std::endl;
	std::cout << "        " << Reflection::TypeResolver<std::map<size_t,std::vector<std::array<char,3>>>>::GetName() << std::endl;

	std::cout << "ID of std::string should be: 35c5c55b" << std::endl;
	std::cout << "ID of void should be: 7c9faa57" << std::endl;
	std::cout << "ID of std::list<std::array<RecursiveType,5>> should be: b5e4752d" << std::endl;

	std::cout << "\nCreating streams and loading types" << std::endl;
	std::stringstream Stream;
	SerializedTypeA A1;
	SerializedTypeA A2;
	SerializedTypeB B;
	auto* TA = Reflection::TypeResolver<SerializedTypeA>::Get();
	auto* TB = Reflection::TypeResolver<SerializedTypeB>::Get();

	A1.CharValue = 'x';
	A1.ShortValue = 9999;
	A1.FloatValue = 99.99;

	std::cout << std::boolalpha;

	std::cout << "Serializing A1" << std::endl;
	TA->Serializer->SerializeBinary( &A1, Stream );
	std::cout << "Deserializing A1 to A2" << std::endl;

	const bool SuccessA = TA->Serializer->DeserializeBinary( &A2, Stream );
	std::cout << "Result: " << SuccessA << " " << (A1 == A2) << std::endl;

	Stream.str( "" );
	Stream.clear();

	std::cout << "Serializing A1 again" << std::endl;
	TA->Serializer->SerializeBinary( &A1, Stream );
	std::cout << "Deserializing A1 to B" << std::endl;
	const bool SuccessB = TB->Serializer->DeserializeBinary( &B, Stream );
	std::cout << "Result: " << SuccessB << " " << (B == A1) << std::endl;

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

		MeshComponent* TestMesh = MeshEntity->Get( Mesh );
		TestMesh->Vertices =
		{
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

		MainLoop( CTX );
	}

	Shutdown( CTX );

	CTX.Log->Message(
		l_printf(
			CTX.Temp, "[TempBuffer]{ Current: %i/%i, Peak: %i/%i }",
			CTX.Temp.GetUsed(), CTX.Temp.GetCapacity(), CTX.Temp.GetPeakUsage(), CTX.Temp.GetCapacity()
		)
	);

	return 0;
}
