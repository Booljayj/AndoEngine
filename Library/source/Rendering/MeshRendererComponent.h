#pragma once
#include <GL/glew.h>
#include "EntityFramework/Managers/SimpleComponentManager.h"
#include "Rendering/GLVertexArrayObject.h"
#include "Rendering/Shader.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/VertexData.h"

namespace C
{
	struct MeshRendererComponent
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

		inline void Setup( C::MeshComponent* Mesh )
		{
			VertexCount = static_cast<GLuint>( Mesh->Vertices.size() );

			glGenVertexArrays( 1, &VertexArrayID );
			GL::BindBuffersToVertexArrayObject( VertexArrayID, Mesh->BufferID );
			//cout << GL::DescribeVertexArrayObject( VertexArrayID );
		}

		inline void Teardown()
		{
			glDeleteVertexArrays( 1, &VertexArrayID );
		}
	};

	using MeshRendererComponentManager = TSimpleComponentManager<MeshRendererComponent>;
}
