#include <cassert>
#include "Rendering/RenderingSystem.h"
#include "EntityFramework/Entity.h"
#include <glm/vec3.hpp>

using namespace std;

void PrintGLErrors()
{
	GLenum ErrorCode = glGetError();
	if( ErrorCode != GL_NO_ERROR )
	{
		cerr << "OpenGL Error: " << ErrorCode << endl;
	}
}

namespace S
{
	void RenderingSystem::Update() const
	{
		const auto RenderMeshRendererComponent =
			[this]( C::MeshRenderer* Comp )
			{
				Render( Comp );
			};

		MeshRendererManager->ForEach( RenderMeshRendererComponent );
	}

	void RenderingSystem::Render( const C::MeshRenderer* MeshRendererComp ) const
	{
		if( !MeshRendererComp->IsValid() ) return;

		glBindVertexArray( MeshRendererComp->VertexArrayID );
		glDrawArrays( GL_TRIANGLES, 0, MeshRendererComp->VertexCount );

		PrintGLErrors();
	}
}
