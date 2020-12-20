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
		VkDescriptorSetLayout descriptors = nullptr;
		VkPipelineLayout layout = nullptr;
		VkPipeline pipeline = nullptr;

		inline operator bool() const { return pipeline && layout; }
		inline void Destroy(VkDevice device) {
			if (pipeline) vkDestroyPipeline(device, pipeline, nullptr);
			if (layout) vkDestroyPipelineLayout(device, layout, nullptr);
			if (descriptors) vkDestroyDescriptorSetLayout(device, descriptors, nullptr);
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
			void* data = nullptr;
			vmaMapMemory(allocator, allocation, &data);
			mapped = static_cast<char*>(data);
			return !!mapped;
		}
		/** Unmap the memory that has been previously mapped */
		inline void Unmap(VmaAllocator allocator) {
			vmaUnmapMemory(allocator, allocation);
		}

		/** Write a value at a specific offset in the buffer */
		template<typename T>
		inline void Write(T const& value, size_t offset) const { memcpy(mapped + offset, &value, sizeof(T)); }
		template<typename T>
		inline void Write(TArrayView<T const> values, size_t offset) const { memcpy(mapped + offset, values.begin(), sizeof(T) * values.size()); }
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
		template<typename T>
		inline void Write(T const& value, size_t offset) const { memcpy(mapped + offset, &value, sizeof(T)); }
		template<typename T>
		inline void Write(TArrayView<T const> values, size_t offset) const { memcpy(mapped + offset, values.begin(), sizeof(T) * values.size()); }
	};

	/** Holds a vulkan buffer that will be destroyed at the end of the scope unless released */
	struct UniqueVulkanBuffer {
		VkBuffer buffer = nullptr;
		VmaAllocation allocation = nullptr;
		VmaAllocator allocator = nullptr;
		VkDeviceSize capacity = 0;

		UniqueVulkanBuffer() = default;
		UniqueVulkanBuffer(UniqueVulkanBuffer&& other);
		~UniqueVulkanBuffer();

		operator bool() const { return buffer && allocation; }
		VulkanBuffer Release();
	};

	/** Create a buffer */
	UniqueVulkanBuffer CreateBuffer(VmaAllocator allocator, size_t capacity, VkBufferUsageFlags bufferUsage, VmaMemoryUsage allocationUsage);
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

		inline operator bool() const { return !!buffer; }
		inline void Destroy(VmaAllocator allocator) {
			buffer.Destroy(allocator);
		}
	};

	/** The results of creating the resources for a new mesh */
	struct VulkanMeshCreationResults {
		VkCommandBuffer commands;
		VulkanBuffer staging;
	};

	/** Holds resources related to uniforms */
	struct VulkanUniformResources {
		VulkanMappedBuffer uniforms;
		size_t elementSize = 0;

		inline operator bool() const { return uniforms && elementSize > 0; }

		inline void Destroy(VmaAllocator allocator) const {
			uniforms.Destroy(allocator);
		}
		inline void Destroy(VmaAllocator allocator) {
			uniforms.Destroy(allocator);
			elementSize = 0;
		}

		EResourceModifyResult Reserve(VmaAllocator allocator, size_t newSize, size_t newCapacity);

		template<typename T>
		inline void Write(T const& value, size_t offset) const { uniforms.Write(value, offset); }
	};

	/** Holds resources and the descriptor set that describes them */
	struct VulkanUniforms {
		VkDescriptorSet set = nullptr;
		VulkanUniformResources resources;

		inline operator bool() const { return set && resources; }

		/** Update the descriptor set so it describes the resources */
		template<bool UseDynamicOffsets>
		inline void UpdateDescriptors(VkDevice device) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = resources.uniforms.buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = resources.elementSize;

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
