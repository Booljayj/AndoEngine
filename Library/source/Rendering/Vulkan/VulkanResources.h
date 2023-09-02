#pragma once
#include "Engine/ArrayView.h"
#include "Rendering/Vulkan/Buffers.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
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
}
