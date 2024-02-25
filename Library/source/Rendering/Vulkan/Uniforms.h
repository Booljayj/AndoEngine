#pragma once
#include "Rendering/Vulkan/Buffers.h"

namespace Rendering{
	namespace Concepts {
		template<typename T>
		concept UniformType =
			Concepts::BufferWritable<T> and
			requires {
				{ T::GetBinding() } -> std::convertible_to<VkDescriptorSetLayoutBinding>;
			};
	}

	/** Holds buffered elements and the descriptor set that describes them. Elements are contiguous within the buffer. */
	template<Concepts::UniformType T>
	struct Uniforms {
	public:
		/** Get the byte offset of the element at the specified index */
		static constexpr inline uint32_t OffsetOf(size_t index) { return index * sizeof(T); }

		Uniforms(VkDevice device, VkDescriptorSet set, size_t numElements, VmaAllocator allocator)
			: device(device), set(set)
			, ubo(allocator, numElements * sizeof(T), BufferUsage::Uniform, MemoryUsage::CPU_to_GPU)
		{
			UpdateDescriptorSet();
		}

		inline operator VkDescriptorSet() const { return set; }

		inline size_t GetNumElements() const { return ubo.GetSize() / sizeof(T); }

		/** Reserve a new capacity for this buffer. No effect if the current capacity is already larger than the new capacity. */
		void Reserve(size_t newNumElements) {
			ubo.Reserve(newNumElements * sizeof(T));
			UpdateDescriptorSet();
		}

		/** Ensure all previous writes are available to the GPU. Must be called after writing and before submitting the buffer. */
		void Flush() const { ubo.Flush(); }
		
		/** Write a value at a specific index in the buffer. Returns the byte offset where the value was written. */
		inline uint32_t Write(T const& value, size_t index = 0) const { ubo.WriteSingle(value, OffsetOf(index)); return OffsetOf(index); }
		
	private:
		VkDevice device = nullptr;
		VkDescriptorSet set = nullptr;
		MappedBuffer ubo;

		void UpdateDescriptorSet() const {
			VkDescriptorSetLayoutBinding const layout = T::GetBinding();

			VkDescriptorBufferInfo const bufferInfo{
				.buffer = ubo,
				.offset = 0,
				.range = VK_WHOLE_SIZE,
			};

			VkWriteDescriptorSet const write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = set,
				.dstBinding = layout.binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = layout.descriptorType,
				.pImageInfo = nullptr, // Optional
				.pBufferInfo = &bufferInfo,
				.pTexelBufferView = nullptr, // Optional
			};
			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
		}
	};
}
