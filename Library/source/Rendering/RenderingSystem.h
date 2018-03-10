#pragma once
#include "Engine/UtilityMacros.h"
#include "EntityFramework/ComponentInfo.h"
#include "Rendering/MeshRendererComponent.h"

class RenderingSystem
{
private:
	const MeshRendererComponentManager* MeshRendererManager;

public:
	bool Startup( CTX_ARG, const MeshRendererComponentManager* InMeshRendererManager );
	bool Shutdown( CTX_ARG ) { return true; }

	void RenderFrame( float InterpolationAlpha ) const;
	static void RenderComponent( const MeshRendererComponent* MeshRenderer );
};
