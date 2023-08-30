#include "Rendering/Vulkan/VulkanResources.h"
#include "Engine/Temporary.h"

namespace Rendering {
	Buffer::Writer::Writer(Buffer const& source)
		: allocator(source.allocator), allocation(source.allocation)
	{
		if (vmaMapMemory(allocator, allocation, reinterpret_cast<void**>(&mapped)) == VK_SUCCESS || !mapped) {
			throw std::runtime_error{ "Unable to map buffer. This may mean the wrong VmaMemoryUsage was used to create it." };
		}
	}

	Buffer::Writer::~Writer() {
		vmaFlushAllocation(allocator, allocation, 0, VK_WHOLE_SIZE);
		vmaUnmapMemory(allocator, allocation);
		mapped = nullptr;
	}

	Buffer::Buffer(VmaAllocator inAllocator, size_t inCapacity, VkBufferUsageFlags inBufferUsage, VmaMemoryUsage inAllocationUsage)
		: allocator(inAllocator), capacity(inCapacity), bufferUsage(inBufferUsage), allocationUsage(inAllocationUsage)
	{
		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = capacity;
		bufferCI.usage = bufferUsage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI{};
		allocCI.usage = allocationUsage;

		VmaAllocationInfo info{};
		if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, &info) != VK_SUCCESS || !buffer || !allocation) {
			throw std::runtime_error{ t_printf("Unable to allocate %i bytes for buffer", capacity).data() };
		}
		capacity = info.size;
	}

	Buffer::Buffer(Buffer&& other) noexcept {
		std::swap(allocator, other.allocator);
		std::swap(capacity, other.capacity);
		std::swap(bufferUsage, other.bufferUsage);
		std::swap(allocationUsage, other.allocationUsage);

		std::swap(allocation, other.allocation);
		std::swap(buffer, other.buffer);
	}

	Buffer::~Buffer() {
		if (allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			allocator = nullptr;
		}
	}

	void Buffer::Reserve(VkDeviceSize newCapacity) {
		if (!allocator) throw std::runtime_error{ "Cannot resize buffer, no allocator present" };
		
		if (newCapacity > capacity) {
			capacity = newCapacity;

			vmaDestroyBuffer(allocator, buffer, allocation);

			VkBufferCreateInfo bufferCI{};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.size = capacity;
			bufferCI.usage = bufferUsage;
			bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocCI{};
			allocCI.usage = allocationUsage;

			VmaAllocationInfo info{};
			if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, &info) != VK_SUCCESS || !buffer || !allocation) {
				throw std::runtime_error{ t_printf("Unable to allocate %i bytes for buffer", capacity).data() };
			}
			capacity = info.size;
		}
	}

	MappedBuffer::MappedBuffer(VmaAllocator inAllocator, size_t inCapacity, VkBufferUsageFlags inBufferUsage, VmaMemoryUsage inAllocationUsage)
		: allocator(inAllocator), capacity(inCapacity), bufferUsage(inBufferUsage), allocationUsage(inAllocationUsage) 
	{
		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = capacity;
		bufferCI.usage = bufferUsage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCI{};
		allocCI.usage = allocationUsage;
		allocCI.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo info{};
		if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, &info) != VK_SUCCESS || !buffer || !allocation) {
			throw std::runtime_error{ t_printf("Unable to allocate %i bytes for buffer", capacity).data() };
		}
		capacity = info.size;
		mapped = static_cast<char*>(info.pMappedData);

		if (!mapped) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			throw std::runtime_error{ "Unable to map memory allocated for MappedBuffer. This may mean the wrong VmaMemoryUsage was provided." };
		}
	}

	MappedBuffer::MappedBuffer(MappedBuffer&& other) noexcept {
		std::swap(allocator, other.allocator);
		std::swap(capacity, other.capacity);
		std::swap(bufferUsage, other.bufferUsage);
		std::swap(allocationUsage, other.allocationUsage);

		std::swap(allocation, other.allocation);
		std::swap(buffer, other.buffer);
		std::swap(mapped, other.mapped);
	}

	MappedBuffer::~MappedBuffer() {
		if (allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			allocator = nullptr;
		}
	}

	void MappedBuffer::Reserve(VkDeviceSize newCapacity) {
		if (!allocator) throw std::runtime_error{ "Cannot resize buffer, no allocator present" };

		if (newCapacity > capacity) {
			capacity = newCapacity;

			vmaDestroyBuffer(allocator, buffer, allocation);

			VkBufferCreateInfo bufferCI{};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.size = capacity;
			bufferCI.usage = bufferUsage;
			bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocCI{};
			allocCI.usage = allocationUsage;
			allocCI.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;

			VmaAllocationInfo info{};
			if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, &info) != VK_SUCCESS || !buffer || !allocation) {
				throw std::runtime_error{ t_printf("Unable to allocate %i bytes for buffer", capacity).data() };
			}
			capacity = info.size;
			mapped = static_cast<char*>(info.pMappedData);

			if (!mapped) {
				vmaDestroyBuffer(allocator, buffer, allocation);
				throw std::runtime_error{ "Unable to map memory allocated for MappedBuffer. This may mean the wrong VmaMemoryUsage was used to create it." };
			}
		}
	}

	void MappedBuffer::Flush() const {
		if (vmaFlushAllocation(allocator, allocation, 0, VK_WHOLE_SIZE) != VK_SUCCESS) {
			throw std::runtime_error{ "Unable to flush mapped buffer" };
		}
	}

	MeshResources::MeshResources(VmaAllocator inAllocator, size_t inCapacity)
		: buffer(inAllocator, inCapacity, BufferUsageBits::VertexBuffer | BufferUsageBits::IndexBuffer | BufferUsageBits::TransferDst, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
	{}

	MeshResources::MeshResources(MeshResources&& other) noexcept
		: buffer(std::move(other.buffer))
	{
		std::swap(offset.vertex, other.offset.vertex);
		std::swap(offset.index, other.offset.index);
		std::swap(size.vertices, other.size.vertices);
		std::swap(size.indices, other.size.indices);
		std::swap(indexType, other.indexType);
	}
}
