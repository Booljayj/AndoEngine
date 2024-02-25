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

	/** Aliases for values in the VmaMemoryUsage enum */
	namespace MemoryUsage {
		constexpr VmaMemoryUsage Unknown = VMA_MEMORY_USAGE_UNKNOWN;
		constexpr VmaMemoryUsage GPU_Only = VMA_MEMORY_USAGE_GPU_ONLY;
		constexpr VmaMemoryUsage CPU_Only = VMA_MEMORY_USAGE_CPU_ONLY;
		constexpr VmaMemoryUsage CPU_to_GPU = VMA_MEMORY_USAGE_CPU_TO_GPU;
		constexpr VmaMemoryUsage GPU_to_CPU = VMA_MEMORY_USAGE_GPU_TO_CPU;
		constexpr VmaMemoryUsage CPU_Copy = VMA_MEMORY_USAGE_CPU_COPY;
		constexpr VmaMemoryUsage LazilyAllocated = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
	}

	namespace Concepts {
		template<typename T>
		concept BufferWritable = std::is_trivially_copyable_v<T> and std::is_trivially_destructible_v<T>;
	}

	/** A buffer and the memory allocation for it */
	struct Buffer {
		/** A scoped writing context for a vulkan buffer. Memory is mapped for CPU access while this exists. */
		struct Writer {
			Writer(Writer const&) = delete;
			Writer(Writer&&) = delete;
			~Writer();

			/** Write a value at a specific offset in the buffer */
			template<Concepts::BufferWritable T>
			inline void WriteSingle(T const& value, size_t offset) const { memcpy(mapped + offset, &value, sizeof(T)); }
			/** Write a collection of values at a specific offset in the buffer */
			template<Concepts::BufferWritable T>
			inline void WriteMultiple(std::span<T const> values, size_t offset) const { memcpy(mapped + offset, values.begin(), values.size() * sizeof(T)); }

		private:
			friend struct Buffer;

			VmaAllocator allocator = nullptr;
			VmaAllocation allocation = nullptr;
			char* mapped = nullptr;

			Writer(Buffer const&);
		};

		Buffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage);
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
		MappedBuffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage);
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
		template<Concepts::BufferWritable T>
		inline void WriteSingle(T const& value, size_t offset) const { memcpy(mapped + offset, &value, sizeof(T)); }
		/** Write a collection of values at a specific offset in the buffer */
		template<Concepts::BufferWritable T>
		inline void WriteMultiple(std::span<T const> values, size_t offset) const {
			auto const bytes = std::as_bytes(values);
			memcpy(mapped + offset, bytes.data(), bytes.size());
		}

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