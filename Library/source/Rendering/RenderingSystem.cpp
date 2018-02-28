#include <cassert>
#include "Rendering/RenderingSystem.h"
#include "EntityFramework/Entity.h"
#include <glm/vec3.hpp>

using namespace std;

namespace S
{
	bool RenderingSystem::Startup( CTX_ARG, const C::MeshRendererComponentManager* InMeshRendererManager )
	{
		MeshRendererManager = InMeshRendererManager;
		return !!MeshRendererManager;
	}

	void RenderingSystem::RenderFrame( float InterpolationAlpha ) const
	{
		MeshRendererManager->ForEach( &RenderingSystem::RenderComponent );
	}

	void RenderingSystem::RenderComponent( const C::MeshRendererComponent* MeshRenderer )
	{
		if( !MeshRenderer->IsValid() ) return;

		glBindVertexArray( MeshRenderer->VertexArrayID );
		glDrawArrays( GL_TRIANGLES, 0, MeshRenderer->VertexCount );

		GLenum ErrorCode = glGetError();
		if( ErrorCode != GL_NO_ERROR )
		{
			cerr << "OpenGL Error: " << ErrorCode << endl;
		}
	}
}
