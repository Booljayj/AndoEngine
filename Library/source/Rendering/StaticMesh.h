#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/Reflection.h"
#include "Engine/SmartPointers.h"
#include "Engine/Variant.h"
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
		DECLARE_STRUCT_REFLECTION_MEMBERS(StaticMesh, Resources::Resource);
		using Resources::Resource::Resource;

		FormattedVertices vertices;
		FormattedIndices indices;

		std::shared_ptr<MeshResources> objects;
	};
}

REFLECT(Rendering::StaticMesh, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(Rendering::StaticMesh);
DEFINE_DEFAULT_YAML_SERIALIZATION(Rendering::StaticMesh);
