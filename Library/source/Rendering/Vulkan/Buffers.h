#pragma once
#include "Engine/StandardTypes.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Aliases for values in the VkBufferUsageFlagBits enum */
	namespace BufferUsage {
		constexpr VkBufferUsageFlagBits TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		constexpr VkBufferUsageFlagBits TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		constexpr VkBufferUsageFlagBits Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		constexpr VkBufferUsageFlagBits Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		constexpr VkBufferUsageFlagBits Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	/** A buffer and the memory allocation for it */
	struct Buffer {
		/** A scoped writing context for a vulkan buffer. Memory is mapped for CPU access while this exists. */
		struct Writer {
			Writer(Writer const&) = delete;
			Writer(Writer&&) = delete;
			~Writer();

			/** Write a value at a specific offset in the buffer */
			inline void Write(void const* begin, size_t size, size_t offset) const { memcpy(mapped + offset, begin, size); }
			template<typename T>
			inline void WriteValue(T const& value, size_t offset) const { Write(&value, sizeof(T), offset); }
			template<typename T>
			inline void WriteArray(std::span<T> values, size_t offset) const { Write(values.begin(), values.size() * sizeof(T), offset); }

		private:
			friend struct Buffer;

			VmaAllocator allocator = nullptr;
			VmaAllocation allocation = nullptr;
			char* mapped = nullptr;

			Writer(Buffer const&);
		};

		Buffer(VmaAllocator inAllocator, size_t inSize, VkBufferUsageFlags inBufferUsage, VmaMemoryUsage inAllocationUsage);
		Buffer(Buffer const&) = delete;
		Buffer(Buffer&&) noexcept;
		~Buffer();

		inline operator VkBuffer() const { return buffer; }

		/** The size of the buffer in bytes. The total memory allocation for this buffer may be larger. */
		VkDeviceSize GetSize() const { return size; }
		/** Reserve a new size for this buffer. No effect if the current size is already larger than the new size. */
		void Reserve(VkDeviceSize newSize);

		/** Create a writer object that can be used to write to this buffer from the CPU */
		Writer CreateWriter() const { return Writer(*this); }

	private:
		VmaAllocator allocator = nullptr;
		VkDeviceSize size = 0;
		VkBufferUsageFlags bufferUsage = 0;
		VmaMemoryUsage allocationUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;

		VmaAllocation allocation = nullptr;
		VkBuffer buffer = nullptr;
	};

	/** A buffer and the memory allocation for it. Memory is persistently mapped for CPU access. */
	struct MappedBuffer {
		MappedBuffer(VmaAllocator inAllocator, size_t inSize, VkBufferUsageFlags inBufferUsage, VmaMemoryUsage inAllocationUsage);
		MappedBuffer(MappedBuffer const&) = delete;
		MappedBuffer(MappedBuffer&&) noexcept;
		~MappedBuffer();

		inline operator VkBuffer() const { return buffer; }

		/** The size of the buffer in bytes. The total memory allocation for this buffer may be larger. */
		VkDeviceSize GetSize() const { return size; }
		/** Reserve a new size for this buffer. No effect if the current size is already larger than the new size. */
		void Reserve(VkDeviceSize newSize);

		/** Ensure all previous writes are available to the GPU. Must be called after writing and before submitting the buffer. */
		void Flush() const;

		/** Write a value at a specific offset in the buffer */
		inline void Write(void const* begin, size_t size, size_t offset) const { memcpy(mapped + offset, begin, size); }
		template<typename T>
		inline void WriteValue(T const& value, size_t offset) const { Write(&value, sizeof(T), offset); }
		template<typename T>
		inline void WriteArray(std::span<T> values, size_t offset) const { Write(values.begin(), values.size() * sizeof(T), offset); }

	private:
		VmaAllocator allocator = nullptr;
		VkDeviceSize size = 0;
		VkBufferUsageFlags bufferUsage = 0;
		VmaMemoryUsage allocationUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;

		VmaAllocation allocation = nullptr;
		VkBuffer buffer = nullptr;
		char* mapped = nullptr;
	};
}