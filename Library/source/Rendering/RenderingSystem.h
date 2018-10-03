#pragma once
#include <memory>
#include "Engine/Context.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/EntityFilter.h"

struct TransformComponent;
struct MeshRendererComponent;
struct EntityCollectionSystem;

class RenderingSystem
{
private:
	static constexpr size_t FILTER_SIZE = 2;
	std::shared_ptr<EntityFilter<FILTER_SIZE>> Filter;
	TComponentHandle<TransformComponent> TransformHandle;
	TComponentHandle<MeshRendererComponent> MeshRendererHandle;

public:
	bool Startup( CTX_ARG,
		EntityCollectionSystem* EntityCollection,
		TComponentInfo<TransformComponent>* Transform,
		TComponentInfo<MeshRendererComponent>* MeshRenderer
	);
	bool Shutdown( CTX_ARG ) { return true; }

	void RenderFrame( float InterpolationAlpha ) const;
	static void RenderComponent( MeshRendererComponent const* MeshRenderer );
};
