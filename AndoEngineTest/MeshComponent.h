//
//  MeshComponent.h
//  AndoEngine
//
//  Created by Justin Bool on 8/6/17.
//
//

#pragma once

#include <cassert>

#include "GL/glew.h"
#include "glm/vec3.hpp"

#include "TCompManager.h"
#include "TCompInfo.h"

#include "GLVertexData.h"
#include "GLVertexBufferObject.h"

using namespace glm;

namespace C
{
	struct Mesh
	{
		vector<GL::VertexData> Vertices;
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
}
