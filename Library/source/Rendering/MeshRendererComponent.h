#pragma once
#include <GL/glew.h>
#include "EntityFramework/Managers/SimpleComponentManager.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/Shader.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/VertexData.h"

struct MeshRendererComponent {
	GLuint vertexArrayID = 0;
	GLuint vertexCount = 0;

	void OnRetained() {}

	void OnReleased() {
		if (IsValid()) Teardown();
	}

	bool IsValid() const {
		return vertexArrayID != 0 && vertexCount > 0;
	}

	inline void Setup(MeshComponent* mesh) {
		vertexCount = static_cast<GLuint>(mesh->vertices.size());

		glGenVertexArrays(1, &vertexArrayID);
		GL::BindBuffersToVertexArrayObject(vertexArrayID, mesh->bufferID);
		//cout << GL::DescribeVertexArrayObject( vertexArrayID );
	}

	inline void Teardown() {
		glDeleteVertexArrays(1, &vertexArrayID);
	}
};

using MeshRendererComponentManager = TSimpleComponentManager<MeshRendererComponent>;
