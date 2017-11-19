#pragma once

#include <iostream>
#include "GL/glew.h"

#include "Rendering/GLVertexBufferObject.h"
#include "Rendering/GLVertexArrayObject.h"
#include "Rendering/GLShader.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/VertexData.h"

namespace C
{
	struct MeshRenderer
	{
		GLuint VertexArrayID = 0;
		GLuint VertexCount = 0;

		void OnRetained() {}

		void OnReleased()
		{
			if( IsValid() ) Teardown();
		}

		bool IsValid() const
		{
			return VertexArrayID != 0 && VertexCount > 0;
		}

		inline void Setup( C::Mesh* MeshComp )
		{
			VertexCount = static_cast<GLuint>( MeshComp->Vertices.size() );

			glGenVertexArrays( 1, &VertexArrayID );
			GL::BindBuffersToVertexArrayObject( VertexArrayID, MeshComp->BufferID );
			//cout << GL::DescribeVertexArrayObject( VertexArrayID );
		}

		inline void Teardown()
		{
			glDeleteVertexArrays( 1, &VertexArrayID );
		}
	};
}
