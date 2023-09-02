#include "Rendering/Vulkan/VulkanResources.h"
#include "Engine/Temporary.h"

namespace Rendering {
	MeshResources::MeshResources(VmaAllocator inAllocator, size_t inCapacity)
		: buffer(inAllocator, inCapacity, BufferUsage::Vertex | BufferUsage::Index | BufferUsage::TransferDst, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
	{}

	MeshResources::MeshResources(MeshResources&& other) noexcept
		: buffer(std::move(other.buffer))
		, offset({ other.offset.vertex, other.offset.index })
		, size({ other.size.vertices, other.size.indices })
		, indexType(other.indexType)
	{}
}
