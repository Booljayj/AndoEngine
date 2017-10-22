//
//  RenderingSystem.h
//  AndoEngine
//
//  Created by Justin Bool on 8/9/17.
//
//

#pragma once

#include "EntitySystem/CompInfo.h"
#include "EntitySystem/Entity.h"
#include "EntitySystem/EntitySystem.h"
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
		TCompInfo<C::Mesh>* Mesh;
		TCompInfo<C::MeshRenderer>* MeshRenderer;
		TCompInfo<C::ShaderComponent>* Shader;
		TCompInfo<C::ProgramComponent>* ShaderProgram;

		TCompManager<C::MeshRenderer>* MeshRendererManager;

	public:
		RenderingSystem( S::EntitySystem* InEntitySys, TCompInfo<C::Mesh>* InMesh, TCompInfo<C::MeshRenderer>* InMeshRenderer, TCompInfo<C::ShaderComponent>* InShader, TCompInfo<C::ProgramComponent>* InShaderProgram )
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
