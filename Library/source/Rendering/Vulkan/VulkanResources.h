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

		VulkanPipelineResources() {}
		VulkanPipelineResources(VulkanPipelineResources&& other) noexcept { *this = std::move(other); }
		VulkanPipelineResources& operator=(VulkanPipelineResources&& other) noexcept {
			std::swap(descriptorSetLayout, other.descriptorSetLayout);
			std::swap(pipelineLayout, other.pipelineLayout);
			std::swap(pipeline, other.pipeline);
			return *this;
		}

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

	namespace BufferUsageBits {
		constexpr VkBufferUsageFlagBits TransferSrc = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		constexpr VkBufferUsageFlagBits TransferDst = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		constexpr VkBufferUsageFlagBits IndexBuffer = VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		constexpr VkBufferUsageFlagBits VertexBuffer = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
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
			inline void WriteArray(TArrayView<T> values, size_t offset) const { Write(values.begin(), values.size() * sizeof(T), offset); }

		private:
			friend struct Buffer;

			VmaAllocator allocator = nullptr;
			VmaAllocation allocation = nullptr;
			char* mapped = nullptr;

			Writer(Buffer const&);
		};

		Buffer(VmaAllocator inAllocator, size_t inCapacity, VkBufferUsageFlags inBufferUsage, VmaMemoryUsage inAllocationUsage);
		Buffer(Buffer const&) = delete;
		Buffer(Buffer&&) noexcept;
		~Buffer();

		inline operator VkBuffer() const { return buffer; }

		/** The capacity of this buffer in bytes */
		VkDeviceSize GetCapacity() const { return capacity; }
		/** Reserve a new capacity for this buffer. No effect if the current capacity is already larger than the new capacity. */
		void Reserve(VkDeviceSize newCapacity);

		/** Create a writer object that can be used to write to this buffer from the CPU */
		Writer CreateWriter() const { return Writer(*this); }

	private:
		VmaAllocator allocator = nullptr;
		VkDeviceSize capacity = 0;
		VkBufferUsageFlags bufferUsage = 0;
		VmaMemoryUsage allocationUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;

		VmaAllocation allocation = nullptr;
		VkBuffer buffer = nullptr;
	};

	/** A buffer and the memory allocation for it. Memory is persistently mapped for CPU access. */
	struct MappedBuffer {
		MappedBuffer(VmaAllocator inAllocator, size_t inCapacity, VkBufferUsageFlags inBufferUsage, VmaMemoryUsage inAllocationUsage);
		MappedBuffer(MappedBuffer const&) = delete;
		MappedBuffer(MappedBuffer&&) noexcept;
		~MappedBuffer();

		inline operator VkBuffer() const { return buffer; }

		/** The capacity of this buffer in bytes */
		VkDeviceSize GetCapacity() const { return capacity; }
		/** Reserve a new capacity for this buffer. No effect if the current capacity is already larger than the new capacity. */
		void Reserve(VkDeviceSize newCapacity);

		/** Ensure all previous writes are available to the GPU. Must be called after writing and before submitting the buffer. */
		void Flush() const;

		/** Write a value at a specific offset in the buffer */
		inline void Write(void const* begin, size_t size, size_t offset) const { memcpy(mapped + offset, begin, size); }
		template<typename T>
		inline void WriteValue(T const& value, size_t offset) const { Write(&value, sizeof(T), offset); }
		template<typename T>
		inline void WriteArray(TArrayView<T> values, size_t offset) const { Write(values.begin(), values.size() * sizeof(T), offset); }

	private:
		VmaAllocator allocator = nullptr;
		VkDeviceSize capacity = 0;
		VkBufferUsageFlags bufferUsage = 0;
		VmaMemoryUsage allocationUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;

		VmaAllocation allocation = nullptr;
		VkBuffer buffer = nullptr;
		char* mapped = nullptr;
	};

	/** Stores resources related to a mesh */
	struct MeshResources {
		Buffer buffer;

		struct {
			VkDeviceSize vertex = 0;
			VkDeviceSize index = 0;
		} offset;

		struct {
			uint32_t vertices = 0;
			uint32_t indices = 0;
		} size;

		VkIndexType indexType = VK_INDEX_TYPE_MAX_ENUM;

		MeshResources(VmaAllocator inAllocator, size_t inCapacity);
		MeshResources(MeshResources const&) = delete;
		MeshResources(MeshResources&&) noexcept;
	};

	/** A collection of descriptor sets allocated from a descriptor pool */
	template<size_t Size>
	struct DescriptorSets {
	public:
		DescriptorSets(VkDevice device, VkDescriptorPool pool, std::array<VkDescriptorSetLayout, Size> layouts)
		{
			VkDescriptorSetAllocateInfo descriptorSetAI{};
			descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAI.descriptorPool = pool;
			descriptorSetAI.descriptorSetCount = Size;
			descriptorSetAI.pSetLayouts = layouts.data();

			//Descriptor pools are set up with the assumption that sets will not be individually freed.
			//With that in mind, this class just needs to allocate the sets and nothing else.
			if (vkAllocateDescriptorSets(device, &descriptorSetAI, sets.data()) != VK_SUCCESS || !sets[0]) {
				throw std::runtime_error{ "Could not allocate descriptor sets" };
			}
		}
		DescriptorSets(DescriptorSets const&) = delete;
		DescriptorSets(DescriptorSets&& other) = default;

		inline VkDescriptorSet operator[](size_t index) const { return sets[index]; }

	private:
		std::array<VkDescriptorSet, Size> sets;
	};

	/** The type of indexing used with a uniforms object */
	enum struct EUniformsIndexing {
		NonDynamic,
		Dynamic,
	};

	/** Holds resources and the descriptor set that describes them */
	template<typename T, EUniformsIndexing Indexing>
	struct Uniforms {
	public:
		Uniforms(VkDevice inDevice, VkDescriptorSet inSet, uint32_t inBindingIndex, size_t initialNumElements, VmaAllocator allocator)
			: device(inDevice), set(inSet), bindingIndex(inBindingIndex)
			, ubo(allocator, initialNumElements * sizeof(T), VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU)
		{
			UpdateDescriptorSet();
		}

		inline operator VkDescriptorSet() const { return set; }

		inline size_t GetNumElements() const { return ubo.GetCapacity() / sizeof(T); }

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
		uint32_t bindingIndex = 0;
		MappedBuffer ubo;

		void UpdateDescriptorSet() const {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = ubo;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = set;
			descriptorWrite.dstBinding = bindingIndex;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = Indexing == EUniformsIndexing::Dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	};
}
