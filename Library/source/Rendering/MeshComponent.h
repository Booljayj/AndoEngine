#pragma once
#include <vector>
#include <GL/glew.h>
#include "EntityFramework/Managers/SimpleComponentManager.h"
#include "Rendering/EBuffer.enum.h"
#include "Rendering/VertexData.h"

struct MeshComponent {
	std::vector<GL::VertexData> vertices;
	GLuint bufferID[GL::EBuffer::Count()] = {0};

	void OnRetained() {}

	void OnReleased() {
		if (bufferID[0] != 0) DestroyBuffers();
	}

	void CreateBuffers() {
		glGenBuffers(GL::EBuffer::Count(), &bufferID[0]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferID[GL::EBuffer::VertexData]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(vertices)::value_type) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	}

	void DestroyBuffers() {
		glDeleteBuffers(GL::EBuffer::Count(), &bufferID[0]);
	}
};

using MeshComponentManager = TSimpleComponentManager<MeshComponent>;
