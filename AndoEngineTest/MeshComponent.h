//
//  MeshComponent.h
//  AndoEngine
//
//  Created by Justin Bool on 8/6/17.
//
//

#pragma once

#include "TCompManager.h"
#include "TCompInfo.h"
#include "GL/glew.h"
#include "glm/vec3.hpp"

using namespace glm;

namespace C
{
	struct Mesh
	{
		enum {
			VertexBuffer = 0,
			ColorBuffer,
			NormalBuffer,

			MaxBufferSize,
		};

		Mesh()
		{
			//Default screen triangle
			Vertices = {
				vec3{ -1.0f, -1.0f, 0.0f },
				vec3{ 1.0f, -1.0f, 0.0f },
				vec3{ 0.0f,  1.0f, 0.0f },
			};
//			VertData = {
//				-1.0f, -1.0f, 0.0f,
//				1.0f, -1.0f, 0.0f,
//				0.0f, 1.0f, 0.0f,
//			};
		}

		vector<vec3> Vertices;
		GLfloat VertData[9] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
		};
		GLuint BufferID[MaxBufferSize] = {0};

		void OnRetained()
		{
			glGenBuffers( 1, &BufferID[VertexBuffer] );
		}

		void OnReleased()
		{
			glDeleteBuffers( 1, &BufferID[VertexBuffer] );
		}
	};
}
