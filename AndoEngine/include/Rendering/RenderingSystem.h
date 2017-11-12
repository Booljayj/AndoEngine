//
//  RenderingSystem.h
//  AndoEngine
//
//  Created by Justin Bool on 8/9/17.
//
//

#pragma once

#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/Entity.h"
#include "EntityFramework/EntitySystem.h"
#include "AndoEngine/BasicComponents.h"
#include "MeshComponent.h"
#include "MeshRendererComponent.h"

#include "GLShader.h"

namespace S
{
	class RenderingSystem
	{
	private:
		S::EntitySystem* EntitySys;
		TComponentInfo<C::Mesh>* Mesh;
		TComponentInfo<C::MeshRenderer>* MeshRenderer;
		TComponentInfo<C::ShaderComponent>* Shader;
		TComponentInfo<C::ProgramComponent>* ShaderProgram;

		TComponentManager<C::MeshRenderer>* MeshRendererManager;

	public:
		RenderingSystem( S::EntitySystem* InEntitySys, TComponentInfo<C::Mesh>* InMesh, TComponentInfo<C::MeshRenderer>* InMeshRenderer, TComponentInfo<C::ShaderComponent>* InShader, TComponentInfo<C::ProgramComponent>* InShaderProgram )
		: EntitySys( InEntitySys )
		, Mesh( InMesh )
		, MeshRenderer( InMeshRenderer )
		, Shader( InShader )
		, ShaderProgram( InShaderProgram )
		, MeshRendererManager( InMeshRenderer->GetTypedManager() )
		{}

		void Update() const;
		void Render( const C::MeshRenderer* MeshRendererComp ) const;
	};
}
