#pragma once
#include "Engine/ArrayView.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	enum class EResourceModifyResult {
		Unmodified,
		Modified,
		Error,
	};

	/** Stores resources related to a graphics pipeline */
	struct VulkanPipelineResources {
		VkDescriptorSetLayout descriptorSetLayout = nullptr;
		VkPipelineLayout pipelineLayout = nullptr;
		VkPipeline pipeline = nullptr;

		inline operator bool() const { return pipeline && pipelineLayout; }

		inline void Destroy(VkDevice device) const {
			if (pipeline) vkDestroyPipeline(device, pipeline, nullptr);
			if (pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		}
		inline void Destroy(VkDevice device) {
			if (pipeline) vkDestroyPipeline(device, pipeline, nullptr);
			if (pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
			*this = VulkanPipelineResources{};
		}
	};

	/** A buffer and the memory allocation for it */
	struct VulkanBuffer {
		VkBuffer buffer = nullptr;
		VmaAllocation allocation = nullptr;
		VkDeviceSize capacity = 0;
		char* mapped = nullptr;

		inline operator bool() const { return buffer && allocation; }
		inline operator VkBuffer() const { return buffer; }
		inline operator VmaAllocation() const { return allocation; }

		inline void Destroy(VmaAllocator allocator) const {
			vmaDestroyBuffer(allocator, buffer, allocation);
		}
		inline void Destroy(VmaAllocator allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			*this = VulkanBuffer{};
		}

		/** Map the memory for CPU access. Returns true if successful. */
		bool Map(VmaAllocator allocator) {
			return vmaMapMemory(allocator, allocation, reinterpret_cast<void**>(&mapped)) == VK_SUCCESS;
		}
		/** Unmap the memory that has been previously mapped */
		inline void Unmap(VmaAllocator allocator) {
			vmaUnmapMemory(allocator, allocation);
		}

		/** Write a value at a specific offset in the buffer */
		inline void Write(void const* begin, size_t size, size_t offset) const { memcpy(mapped + offset, begin, size); }
		template<typename T>
		inline void WriteValue(T const& value, size_t offset) const { Write(&value, sizeof(T), offset); }
		template<typename T>
		inline void WriteArray(TArrayView<T> values, size_t offset) const { Write(values.begin(), values.size() * sizeof(T), offset); }
	};

	/** A buffer and the memory allocation for it. Memory is persistently mapped for CPU access. */
	struct VulkanMappedBuffer {
		VkBuffer buffer = nullptr;
		VmaAllocation allocation = nullptr;
		VkDeviceSize capacity = 0;
		char* mapped = nullptr;

		inline operator bool() const { return buffer && allocation && capacity > 0 && mapped; }
		inline operator VkBuffer() const { return buffer; }
		inline operator VmaAllocation() const { return allocation; }

		inline void Destroy(VmaAllocator allocator) const {
			vmaDestroyBuffer(allocator, buffer, allocation);
		}
		inline void Destroy(VmaAllocator allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			*this = VulkanMappedBuffer{};
		}

		/** Write a value at a specific offset in the buffer */
		inline void Write(void const* begin, size_t size, size_t offset) const { memcpy(mapped + offset, begin, size); }
		template<typename T>
		inline void WriteValue(T const& value, size_t offset) const { Write(&value, sizeof(T), offset); }
		template<typename T>
		inline void WriteArray(TArrayView<T> values, size_t offset) const { Write(values.begin(), values.size() * sizeof(T), offset); }
	};

	/** Create a buffer */
	VulkanBuffer CreateBuffer(VmaAllocator allocator, size_t capacity, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage);
	VulkanMappedBuffer CreateMappedBuffer(VmaAllocator allocator, size_t capacity, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage);

	/** Stores resources related to a mesh */
	struct VulkanMeshResources {
		VulkanBuffer buffer;

		struct {
			VkDeviceSize vertex;
			VkDeviceSize index;
		} offset;

		struct {
			uint32_t vertices;
			uint32_t indices;
		} size;

		inline operator bool() const { return buffer && size.vertices > 0 && size.indices > 0; }

		inline void Destroy(VmaAllocator allocator) const {
			buffer.Destroy(allocator);
		}
		inline void Destroy(VmaAllocator allocator) {
			buffer.Destroy(allocator);
			*this = VulkanMeshResources{};
		}
	};

	/** Holds resources and the descriptor set that describes them */
	struct VulkanUniforms {
		VkDescriptorSet set = nullptr;
		VulkanMappedBuffer ubo;
		size_t elementSize = 0;

		inline operator bool() const { return set && ubo && elementSize > 0; }

		inline void Destroy(VmaAllocator allocator) const {
			ubo.Destroy(allocator);
		}
		inline void Destroy(VmaAllocator allocator) {
			ubo.Destroy(allocator);
			elementSize = 0;
		}

		/** Reserve a certain amount of space in the ubo buffer */
		EResourceModifyResult Reserve(VmaAllocator allocator, size_t newElementSize, size_t newNumElements);

		/** Update the descriptor set so it describes the resources */
		template<bool UseDynamicOffsets>
		inline void UpdateDescriptors(VkDevice device) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = ubo.buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = elementSize;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = set;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = UseDynamicOffsets ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	};
}
