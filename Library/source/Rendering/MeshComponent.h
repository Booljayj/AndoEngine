#pragma once
#include "Engine/StandardTypes.h"
#include "Rendering/Vertex.h"
#include "Rendering/Vulkan/VulkanResources.h"

namespace Rendering {
	struct MeshComponent {
		/** Vertices for this mesh */
		std::vector<Vertex_Simple> vertices;
		/** Vertex indices for this mesh */
		std::vector<uint32_t> indices;

		/** Rendering resources for this mesh */
		VulkanMeshResources resources;

		//@todo Use this type and a known vertex count to allocate a block of arbitrary memory for vertex data. Allow this to be changed.
		void Init(EVertexType type, size_t numVertices, size_t numIndices);
	};
}
