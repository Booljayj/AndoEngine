#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/GLM.h"
#include "Engine/Threads.h"
#include "Engine/SmartPointers.h"
#include "Rendering/UniformTypes.h"
#include "Rendering/Vulkan/Descriptors.h"
#include "Rendering/Vulkan/Fence.h"
#include "Rendering/Vulkan/GraphicsQueue.h"
#include "Rendering/Vulkan/Semaphore.h"
#include "Rendering/Vulkan/Uniforms.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct GraphicsPipelineResources;
	struct MeshResources;
	struct ResourcesCollection;
	struct ViewParameters;

	struct StaticMeshParameters {
		std::shared_ptr<GraphicsPipelineResources> pipeline_resources;
		std::shared_ptr<MeshResources> mesh_resources;
	};

	struct ThreadMeshCollection {
		std::vector<StaticMeshParameters> static_meshes;
	};

	/** Rendering resources used by a single view */
	struct ViewContext {
		struct ConstructionParameters {
			VkDevice device;
			GraphicsQueue graphics;
			VkDescriptorSetLayout global_layout;
			VkDescriptorSetLayout object_layout;
			VmaAllocator allocator;
		};

		struct Culling {
			/** The frustum for this view, calculated based on the camera */
			glm::mat4 frustum;

			/** The context used by each culling thread */
			std::vector<ThreadMeshCollection> thread_mesh_collections;
			/** The current worker threads performing culling */
			std::vector<std::jthread> thread_workers;
		};

		struct Recording {
			GraphicsCommandPool command_pool;
			DescriptorPool descriptor_pool;
			DescriptorSets<2> uniform_descriptor_sets;
			Uniforms<GlobalUniforms, 1> global_uniforms;
			Uniforms<ObjectUniforms, 512> object_uniforms;

			/** The command buffers used by each recording thread. */
			std::vector<VkCommandBuffer> thread_command_buffers;
			/** The current worker threads performing recording */
			std::vector<std::jthread> thread_workers;

			Recording(VkDevice device, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator);
		};

		Culling culling;
		Recording recording;

		ViewContext(ConstructionParameters const& construction_parameters);
		ViewContext(ViewContext const&) = delete;
		ViewContext(ViewContext&&) noexcept = default;

		void PrepareThreads(ViewParameters const& view_parameters);
		void Clear(ResourcesCollection& previous_resources);
	};

	/** Rendering resources used by a single frame */
	struct FrameContext {
		FrameContext(VkDevice device, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator);
		FrameContext(FrameContext const&) = delete;
		FrameContext(FrameContext&&) noexcept = default;

		GraphicsCommandPool command_pool;
		Semaphore image_available_semaphore;
		Fence fence;
		uint32_t current_image_index = 0;

		/** The context used by each view */
		std::vector<ViewContext> views;
		/** The command buffers used by each view */
		std::vector<VkCommandBuffer> view_command_buffers;

		void Prepare(std::span<ViewParameters const> views_parameters, ResourcesCollection& previous_resources);

	private:
		ViewContext::ConstructionParameters view_construction_parameters;
	};
}
