#pragma once
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/Vertex.h"
#include "Rendering/Vulkan/Resources.h"
#include "Resources/Resource.h"

namespace Rendering {
	using Vertices_Simple = std::vector<Vertex_Simple>;
	using Vertices_Complex = std::vector<Vertex_Complex>;
	using FormattedVertices = std::variant<Vertices_Simple, Vertices_Complex>;

	using Indices_Short = std::vector<uint16_t>;
	using Indices_Long = std::vector<uint32_t>;
	using FormattedIndices = std::variant<Indices_Short, Indices_Long>;

	struct StaticMesh : public Resources::Resource {
		REFLECT_STRUCT(StaticMesh, Resources::Resource);
		using Resources::Resource::Resource;

		FormattedVertices vertices;
		FormattedIndices indices;

		std::unique_ptr<MeshResources> objects;
	};
}

REFLECT(Rendering::StaticMesh, Struct);
