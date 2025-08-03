#include "Rendering/Vulkan/Buffers.h"
#include "Engine/TemporaryStrings.h"

namespace Rendering {
	Buffer::Writer::Writer(Buffer const& source)
		: allocator(source.allocator), allocation(source.allocation)
	{
		if (vmaMapMemory(allocator, allocation, reinterpret_cast<void**>(&mapped)) == VK_SUCCESS || !mapped) {
			throw FormatType<std::runtime_error>("Unable to map buffer. This may mean the wrong VmaMemoryUsage was used to create it.");
		}
	}

	Buffer::Writer::~Writer() {
		vmaFlushAllocation(allocator, allocation, 0, VK_WHOLE_SIZE);
		vmaUnmapMemory(allocator, allocation);
		mapped = nullptr;
	}

	Buffer::Buffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage)
		: allocator(allocator), size(size), bufferUsage(bufferUsage), allocationUsage(allocationUsage)
	{
		VkBufferCreateInfo const bufferCI{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = bufferUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		
		VmaAllocationCreateInfo const allocCI{
			.usage = allocationUsage,
		};
		
		if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, nullptr) != VK_SUCCESS || !buffer || !allocation) {
			throw FormatType<std::runtime_error>("Unable to allocate {} bytes for buffer", size);
		}
	}

	Buffer::~Buffer() {
		if (allocator) vmaDestroyBuffer(allocator, buffer, allocation);
	}

	void Buffer::Reserve(VkDeviceSize newSize) {
		if (!allocator) throw FormatType<std::runtime_error>("Cannot resize buffer, no allocator present");

		if (newSize > size) {
			size = newSize;

			vmaDestroyBuffer(allocator, buffer, allocation);

			VkBufferCreateInfo const bufferCI{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = size,
				.usage = bufferUsage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			};

			VmaAllocationCreateInfo const allocCI{
				.usage = allocationUsage,
			};
			
			if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, nullptr) != VK_SUCCESS || !buffer || !allocation) {
				throw FormatType<std::runtime_error>("Unable to allocate {} bytes for buffer", size);
			}
		}
	}

	MappedBuffer::MappedBuffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage)
		: allocator(allocator), size(size), bufferUsage(bufferUsage), allocationUsage(allocationUsage)
	{
		VkBufferCreateInfo const bufferCI{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = bufferUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		
		VmaAllocationCreateInfo const allocCI{
			.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = allocationUsage,
		};
		
		VmaAllocationInfo info{};
		if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, &info) != VK_SUCCESS || !buffer || !allocation) {
			throw FormatType<std::runtime_error>("Unable to allocate {} bytes for buffer", size);
		}
		mapped = static_cast<char*>(info.pMappedData);

		if (!mapped) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			throw FormatType<std::runtime_error>("Unable to map memory allocated for MappedBuffer. This may mean the wrong VmaMemoryUsage was provided.");
		}
	}

	MappedBuffer::~MappedBuffer() {
		if (allocator) vmaDestroyBuffer(allocator, buffer, allocation);
	}

	void MappedBuffer::Reserve(VkDeviceSize newSize) {
		if (!allocator) throw FormatType<std::runtime_error>("Cannot resize buffer, no allocator present");

		if (newSize > size) {
			size = newSize;

			vmaDestroyBuffer(allocator, buffer, allocation);

			VkBufferCreateInfo const bufferCI{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = size,
				.usage = bufferUsage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			};
			
			VmaAllocationCreateInfo const allocCI{
				.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
				.usage = allocationUsage,
			};
			
			VmaAllocationInfo info{};
			if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, &info) != VK_SUCCESS || !buffer || !allocation) {
				throw FormatType<std::runtime_error>("Unable to allocate {0} bytes for buffer", size);
			}
			mapped = static_cast<char*>(info.pMappedData);

			if (!mapped) {
				vmaDestroyBuffer(allocator, buffer, allocation);
				throw FormatType<std::runtime_error>("Unable to map memory allocated for MappedBuffer. This may mean the wrong VmaMemoryUsage was used to create it.");
			}
		}
	}

	void MappedBuffer::Flush() const {
		if (vmaFlushAllocation(allocator, allocation, 0, VK_WHOLE_SIZE) != VK_SUCCESS) {
			throw FormatType<std::runtime_error>("Unable to flush mapped buffer");
		}
	}
}
