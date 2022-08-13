#pragma once
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/Vertex.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Resources/Database.h"
#include "Resources/Resource.h"

namespace Rendering {
	using FormattedVertices = std::variant<
		std::vector<Vertex_Simple>,
		std::vector<Vertex_Complex>
	>;

	using FormattedIndices = std::variant<
		std::vector<uint16_t>,
		std::vector<uint32_t>
	>;

	struct StaticMeshResource : public Resources::Resource {
		REFLECT_STRUCT(StaticMeshResource, Resources::Resource);

		FormattedVertices vertices;
		FormattedIndices indices;
		VulkanMeshResources gpuResources;
	};

	struct StaticMeshResourceDatabase : public Resources::TSparseDatabase<StaticMeshResource, StaticMeshResourceDatabase> {
		friend struct Resources::TSparseDatabase<StaticMeshResource, StaticMeshResourceDatabase>;
		using Resources::TSparseDatabase<StaticMeshResource, StaticMeshResourceDatabase>::TSparseDatabase;

		bool Startup(const RenderingSystem& inRendering);
		bool Shutdown();

		void CollectGarbage();

	private:
		const RenderingSystem* rendering = nullptr;

		void PostCreate(StaticMeshResource& resource);
		void PostLoad(StaticMeshResource& resource);

		void Destroy(Resources::Resource& resource);
	};
}

REFLECT(Rendering::StaticMeshResource, Struct);
