#pragma once
#include "Engine/UtilityMacros.h"
#include "EntityFramework/ComponentInfo.h"
#include "Rendering/MeshRendererComponent.h"

class RenderingSystem
{
private:
	MeshRendererComponentManager const* MeshRendererManager;

public:
	bool Startup( CTX_ARG, MeshRendererComponentManager const* InMeshRendererManager );
	bool Shutdown( CTX_ARG ) { return true; }

	void RenderFrame( float InterpolationAlpha ) const;
	static void RenderComponent( MeshRendererComponent const* MeshRenderer );
};
