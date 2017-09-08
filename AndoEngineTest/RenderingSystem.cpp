//
//  RenderingSystem.cpp
//  AndoEngine
//
//  Created by Justin Bool on 8/11/17.
//
//

#include <cassert>

#include "RenderingSystem.h"
#include "Entity.h"
#include "glm/vec3.hpp"

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
		MeshRendererManager->ForEach(
			[this]( C::MeshRenderer* Comp )
			{
				Render( Comp );
			}
		);
	}

	void RenderingSystem::Render( const C::MeshRenderer* MeshRendererComp ) const
	{
		if( !MeshRendererComp->IsValid() ) return;

		glBindVertexArray( MeshRendererComp->VertexArrayID );
		glDrawArrays( GL_TRIANGLES, 0, MeshRendererComp->VertexCount );

		PrintGLErrors();
	}
}
