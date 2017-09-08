//
//  RenderingSystem.h
//  AndoEngine
//
//  Created by Justin Bool on 8/9/17.
//
//

#pragma once

#include "TCompInfo.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "BasicComponents.h"
#include "MeshComponent.h"
#include "MeshRendererComponent.h"
#include "ShaderComponent.h"
#include "ShaderProgramComponent.h"

namespace S
{
	class RenderingSystem
	{
	private:
		S::EntitySystem* EntitySys;
		TCompInfo<C::Mesh>* Mesh;
		TCompInfo<C::MeshRenderer>* MeshRenderer;
		TCompInfo<C::Shader>* Shader;
		TCompInfo<C::ShaderProgram>* ShaderProgram;

		TCompManager<C::MeshRenderer>* MeshRendererManager;

	public:
		RenderingSystem( S::EntitySystem* InEntitySys, TCompInfo<C::Mesh>* InMesh, TCompInfo<C::MeshRenderer>* InMeshRenderer, TCompInfo<C::Shader>* InShader, TCompInfo<C::ShaderProgram>* InShaderProgram )
		: EntitySys( InEntitySys )
		, Mesh( InMesh )
		, MeshRenderer( InMeshRenderer )
		, Shader( InShader )
		, ShaderProgram( InShaderProgram )
		, MeshRendererManager( InMeshRenderer->GetManager() )
		{}

		void Update() const;
		void Render( const C::MeshRenderer* MeshRendererComp ) const;
	};
}
