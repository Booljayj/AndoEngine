#include <cassert>
#include <glm/vec3.hpp>
#include "Rendering/RenderingSystem.h"

bool RenderingSystem::Startup( CTX_ARG, MeshRendererComponentManager const* InMeshRendererManager )
{
	MeshRendererManager = InMeshRendererManager;
	return !!MeshRendererManager;
}

void RenderingSystem::RenderFrame( float InterpolationAlpha ) const
{
	MeshRendererManager->ForEach( &RenderingSystem::RenderComponent );
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
