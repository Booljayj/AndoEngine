#pragma once

#include "Engine/BasicComponents.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/Entity.h"
#include "EntityFramework/EntitySystem.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/GLShader.h"

namespace S
{
	class RenderingSystem
	{
	private:
		TComponentManager<C::MeshRenderer>* MeshRendererManager;

	public:
		RenderingSystem( TComponentInfo<C::MeshRenderer>* InMeshRenderer )
		: MeshRendererManager( InMeshRenderer->GetTypedManager() )
		{}

		void Update() const;
		void Render( const C::MeshRenderer* MeshRendererComp ) const;
	};
}
