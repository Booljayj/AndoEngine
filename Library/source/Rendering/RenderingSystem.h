#pragma once
#include "Engine/Context.h"
#include "EntityFramework/ComponentInfo.h"
#include "Rendering/MeshRendererComponent.h"

namespace S
{
	class RenderingSystem
	{
	private:
		const C::MeshRendererComponentManager* MeshRendererManager;

	public:
		bool Startup( CTX_ARG, const C::MeshRendererComponentManager* InMeshRendererManager );
		bool Shutdown( CTX_ARG ) { return true; }

		void RenderFrame( float InterpolationAlpha ) const;
		static void RenderComponent( const C::MeshRendererComponent* MeshRenderer );
	};
}
