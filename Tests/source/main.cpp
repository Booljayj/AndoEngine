#include <iostream>
#include <string_view>
#include <GL/glew.h>
#include "Engine/StringID.h"
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

DEFINE_LOG_CATEGORY_STATIC( LogMain, Debug );

bool Startup( CTX_ARG ) {
	TEMP_SCOPE;
	LOG( LogMain, Message, "Starting up all systems..." );

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
	LOG( LogMain, Message, "Shutting down all systems..." );

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

	LOG( LogMain, Message, TERM_Cyan "Hello, World! This is AndoEngine." );
	LOG( LogMain, Message, TERM_Cyan "Compiled with " __VERSION__ " on " __DATE__ );

	std::cout << "\nGlobal types:" << std::endl;
	for( Reflection::TypeInfo const* Info : Reflection::TypeInfo::GetGlobalTypeInfoCollection() ) {
		Reflection::DebugPrint( Info, std::cout, Reflection::FDebugPrintFlags::IncludeMetrics );
	}

	std::cout << "\n\nTuples: " << std::endl;
	std::tuple<char, std::string, size_t> TupleExample = std::make_tuple( 2, std::string{ "example" }, 0xFFFE );
	Reflection::TupleTypeInfo const* ExampleTupleInfo = Reflection::Cast<Reflection::TupleTypeInfo>( Reflection::TypeResolver<decltype( TupleExample )>::Get() );
	if( ExampleTupleInfo ) {
		Reflection::DebugPrint( ExampleTupleInfo, std::cout );
		std::cout << *static_cast<std::string const*>( ExampleTupleInfo->GetValue( &TupleExample, 1 ) ) << std::endl;
	} else {
		std::cout << "Error!" << std::endl;
	}

	std::cout << "\n\nSets: " << std::endl;
	std::set<size_t> SetExample;
	Reflection::SetTypeInfo const* ExampleSetInfo = Reflection::Cast<Reflection::SetTypeInfo>( Reflection::TypeResolver<decltype( SetExample )>::Get() );
	Reflection::DebugPrint( ExampleSetInfo, std::cout );

	Reflection::TypeInfo const* A = Reflection::TypeResolver<std::vector<size_t>>::Get();
	Reflection::DebugPrint( A, std::cout );

	Reflection::TypeInfo const* B = Reflection::TypeResolver<std::unique_ptr<size_t>>::Get();
	Reflection::DebugPrint( B, std::cout );

	std::cout << "ID of std::map<size_t,std::vector<std::array<char,3>>>: " << std::endl;
	std::cout << "        " << Reflection::TypeResolver<std::map<size_t,std::vector<std::array<char,3>>>>::GetID() << std::endl;

	std::cout << "ID of std::string should be: 35c5c55b" << std::endl;
	std::cout << "ID of void should be: 7c9faa57" << std::endl;
	std::cout << "ID of std::list<std::array<RecursiveType,5>> should be: b5e4752d" << std::endl;
	std::cout << std::hex << Reflection::TypeResolver<std::list<std::array<RecursiveType, 5>>>::GetID() << std::endl;

	return 0;
	if( Startup( CTX ) ) {
		LOG( LogMain, Message, "Creating entities" );
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

		MainLoop( CTX );
	}

	Shutdown( CTX );

	LOGF(
		LogMain, Message, "[TempBuffer]{ Current: %i/%i, Peak: %i/%i }",
		CTX.Temp.GetUsed(), CTX.Temp.GetCapacity(), CTX.Temp.GetPeakUsage(), CTX.Temp.GetCapacity()
	);

	return 0;
}
