#pragma once
#include "Rendering/Vulkan/Buffers.h"
#include "Rendering/Vulkan/RenderObjects.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct UniformLayouts;

	enum class EGraphicsLayouts : uint8_t {
		Global,
		Object,
		//Material,
		MAX
	};

	struct VertexInformationViews {
		std::span<VkVertexInputBindingDescription const> bindings;
		std::span<VkVertexInputAttributeDescription const> attributes;
	};

	struct GraphicsPipelineResources : public RenderObjectsBase {
		struct ShaderModules {
			VkShaderModule vertex = nullptr;
			VkShaderModule fragment = nullptr;
		};

		VkPipeline pipeline = nullptr;

		struct {
			VkDescriptorSetLayout set = nullptr;
			VkPipelineLayout pipeline = nullptr;
		} layouts;

		GraphicsPipelineResources(VkDevice device, ShaderModules const& modules, UniformLayouts const& uniforms, VertexInformationViews const& vertex, VkRenderPass pass);
		GraphicsPipelineResources(GraphicsPipelineResources const&) = delete;
		GraphicsPipelineResources(GraphicsPipelineResources&&) noexcept = default;
		~GraphicsPipelineResources();

	private:
		stdext::move_only<VkDevice> device;
	};

	/** Stores resources related to a mesh */
	struct MeshResources : public RenderObjectsBase {
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

		MeshResources(VmaAllocator allocator, size_t capacity);
		MeshResources(MeshResources const&) = delete;
		MeshResources(MeshResources&&) noexcept = default;
	};
}
