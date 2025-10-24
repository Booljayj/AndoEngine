#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/Logging.h"
#include "Engine/MoveOnly.h"
#include "Engine/Optional.h"
#include "Rendering/UniformTypes.h"
#include "Rendering/Views/View.h"
#include "Rendering/Vulkan/Descriptors.h"
#include "Rendering/Vulkan/Device.h"
#include "Rendering/Vulkan/Fence.h"
#include "Rendering/Vulkan/GraphicsQueue.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/ResourcesCollection.h"
#include "Rendering/Vulkan/Semaphore.h"
#include "Rendering/Vulkan/SurfaceQueues.h"
#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Vulkan/UniformLayouts.h"
#include "Rendering/Vulkan/Uniforms.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	//# Frame Organization Structure
	// Frames are organized to ensure they have access to unique resources that they will need for rendering, which will not overlap with the resources needed by other operations and will not be destroyed until the frame is complete.
	// The overall structure is such that each frame will involve one or more views, and each view will involve one or more threads that are recording rendering commands. Like so:
	// - Frame
	//     - View0
	//         - Thread0
	//         - Thread1
	//         - Thread2
	//     - View1
	//         - Thread3
	//         - Thread4
	//     - View2
	//         - Thread5
	//         - Thread6
	//
	// Frames represent everything that will get written to a render target framebuffer, so things like the image index and fence are unique to each frame.
	// Views represent a set of objects that will be rendered to a specific part of the swapchain image, so things like uniforms and primary command buffers are unique to each view.
	// Threads represent a process that will be used to record rendering commands for a subset of objects, so things like secondary command buffers are unique to each thread.

	//@todo It may be better to define this as "StandardUniforms" in a more global place.
	/** Uniforms used for each frame */
	struct FrameUniforms {
		DescriptorSets<2> sets;
		Uniforms<GlobalUniforms, 1> global;
		Uniforms<ObjectUniforms, 512> object;

		FrameUniforms(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout global, VkDescriptorSetLayout object, VmaAllocator allocator);
		FrameUniforms(FrameUniforms const&) = delete;
		FrameUniforms(FrameUniforms&&) noexcept = default;
	};

	/** Rendering resources used by a single thread */
	struct ThreadResources {
		ResourcesCollection resources;

		ThreadResources(size_t num_objects);
		ThreadResources(ThreadResources const&) = delete;
		ThreadResources(ThreadResources&&) noexcept = default;

		void Prepare(size_t num_objects, ResourcesCollection& previous_resources);
	};

	/** Rendering resources used by a single view */
	struct ViewResources {
		GraphicsCommandPool command_pool;
		DescriptorPool descriptor_pool;
		FrameUniforms uniforms;

		std::vector<ThreadResources> threads;
		std::vector<VkCommandBuffer> thread_command_buffers;

		ViewResources(VkDevice device, ViewRenderingParameters view_params, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator);
		ViewResources(ViewResources const&) = delete;
		ViewResources(ViewResources&&) noexcept = default;

		void Prepare(VkDevice device, ViewRenderingParameters view_params, GraphicsQueue graphics, ResourcesCollection& previous_resources);
	};

	/** Rendering resources used by a single frame */
	struct FrameResources {
		FrameResources(VkDevice device, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator);
		FrameResources(FrameResources const&) = delete;
		FrameResources(FrameResources&&) noexcept = default;

		GraphicsCommandPool command_pool;
		Semaphore image_available_semaphore;
		Fence fence;
		uint32_t current_image_index = 0;

		std::vector<ViewResources> views;
		std::vector<VkCommandBuffer> view_command_buffers;

		void Prepare(VkDevice device, std::span<ViewRenderingParameters const> view_params, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator, ResourcesCollection& previous_resources);
	};
}
