#pragma once
#include "Rendering/Vulkan/Buffers.h"

namespace Rendering{
	/** Holds resources and the descriptor set that describes them */
	template<typename T>
	struct Uniforms {
	public:
		Uniforms(VkDevice inDevice, VkDescriptorSet inSet, size_t initialNumElements, VmaAllocator allocator)
			: device(inDevice), set(inSet)
			, ubo(allocator, initialNumElements * sizeof(T), BufferUsage::Uniform, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU)
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

		/** Write a value at a specific index in the buffer */
		inline void Write(T const& value, size_t index) const { ubo.Write(&value, sizeof(T), index * sizeof(T)); }

	private:
		VkDevice device = nullptr;
		VkDescriptorSet set = nullptr;
		MappedBuffer ubo;

		void UpdateDescriptorSet() const {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = ubo;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkDescriptorSetLayoutBinding const layout = T::GetBinding();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = set;
			descriptorWrite.dstBinding = layout.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = layout.descriptorType;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	};
}
