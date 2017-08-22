//
//  MeshRendererComponent.h
//  AndoEngine
//
//  Created by Justin Bool on 8/6/17.
//
//

#pragma once

#include "MeshComponent.h"
#include "ShaderProgramComponent.h"

#include "TCompManager.h"
#include "TCompInfo.h"
#include "GL/glew.h"

namespace C
{
	struct MeshRenderer
	{
		EntityID CurrentMeshEntityID;
		EntityID CurrentShaderProgramID;

		const Mesh* MeshComp;
		const ShaderProgram* ShaderProgramComp;
		GLuint VertexArrayID;

		void OnRetained()
		{
			glGenVertexArrays( 1, &VertexArrayID );
		}

		void OnReleased()
		{
			glDeleteVertexArrays( 1, &VertexArrayID );
		}

		bool IsValid() const
		{
			return VertexArrayID != 0 && MeshComp != nullptr && ShaderProgramComp != nullptr;
		}
	};
}
