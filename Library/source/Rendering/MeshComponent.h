#pragma once
#include <vector>
#include <GL/glew.h>
#include "EntityFramework/Managers/SimpleComponentManager.h"
#include "Rendering/EBuffer.enum.h"
#include "Rendering/VertexData.h"

namespace C
{
	struct Mesh
	{
		std::vector<GL::VertexData> Vertices;
		GLuint BufferID[GL::EBuffer::Count()] = {0};

		void OnRetained() {}

		void OnReleased()
		{
			if( BufferID[0] != 0 ) DestroyBuffers();
		}

		void CreateBuffers()
		{
			glGenBuffers( GL::EBuffer::Count(), &BufferID[0] );
			glBindBuffer( GL_ARRAY_BUFFER, BufferID[GL::EBuffer::VertexData] );
			glBufferData( GL_ARRAY_BUFFER, sizeof(decltype(Vertices)::value_type) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW );
		}

		void DestroyBuffers()
		{
			glDeleteBuffers( GL::EBuffer::Count(), &BufferID[0] );
		}
	};

	using MeshComponentManager = TSimpleComponentManager<Mesh>;
}
