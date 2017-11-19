#pragma once
#include "EntityFramework/ComponentInfo.h"
#include "Rendering/MeshRendererComponent.h"

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
