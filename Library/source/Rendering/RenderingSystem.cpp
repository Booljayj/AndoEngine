#include <cassert>
#include <glm/vec3.hpp>
#include "Rendering/RenderingSystem.h"
#include "Engine/BasicComponents.h"
#include "EntityFramework/EntityCollectionSystem.h"
#include "Rendering/MeshRendererComponent.h"

bool RenderingSystem::Startup( CTX_ARG,
		EntityCollectionSystem* EntityCollection,
		TComponentInfo<TransformComponent>* Transform,
		TComponentInfo<MeshRendererComponent>* MeshRenderer
)
{
	ComponentInfo const* Infos[] = { Transform, MeshRenderer };
	Filter = EntityCollection->MakeFilter( Infos );
	if( Filter )
	{
		TransformHandle = Filter->GetMatchComponentHandle( Transform );
		MeshRendererHandle = Filter->GetMatchComponentHandle( MeshRenderer );
		return true;
	}
	else
	{
		return false;
	}
}

void RenderingSystem::RenderFrame( float InterpolationAlpha ) const
{
	for( EntityFilter<FILTER_SIZE>::FilterMatch const& Match : *Filter ) {
		RenderComponent( Match.Get( MeshRendererHandle ) );
	}
}

void RenderingSystem::RenderComponent( MeshRendererComponent const* MeshRenderer )
{
	if( !MeshRenderer->IsValid() ) return;

	glBindVertexArray( MeshRenderer->VertexArrayID );
	glDrawArrays( GL_TRIANGLES, 0, MeshRenderer->VertexCount );

	GLenum ErrorCode = glGetError();
	if( ErrorCode != GL_NO_ERROR )
	{
		std::cerr << "OpenGL Error: " << ErrorCode << std::endl;
	}
}
