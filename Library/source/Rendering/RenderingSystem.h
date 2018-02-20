#pragma once
#include "EntityFramework/ComponentInfo.h"
#include "Rendering/MeshRendererComponent.h"

namespace S
{
	class RenderingSystem
	{
	private:
		C::MeshRendererComponentManager* _MeshRendererManager;

	public:
		RenderingSystem( C::MeshRendererComponentManager* InMeshRendererManager )
		: _MeshRendererManager( InMeshRendererManager )
		{}

		void operator()( C::MeshRenderer* MeshRendererComp ) const;

		void Update() const;
		void Render( const C::MeshRenderer* MeshRendererComp ) const;
	};
}
