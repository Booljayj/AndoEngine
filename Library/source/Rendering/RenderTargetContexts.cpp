#include "RenderTargetContexts.h"
#include "Rendering/Views/View.h"
#include "Rendering/Vulkan/ResourcesCollection.h"

namespace Rendering {
	constexpr VkDescriptorPoolSize pool_sizes[] = {
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
	};

	ViewContext::Recording::Recording(VkDevice device, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator)
		: command_pool(device, graphics)
		, descriptor_pool(device, pool_sizes, 2)
		, uniform_descriptor_sets(device, descriptor_pool, { global_layout, object_layout })
		, global_uniforms(device, uniform_descriptor_sets[0], 1, allocator)
		, object_uniforms(device, uniform_descriptor_sets[1], 1, allocator)
	{}

	ViewContext::ViewContext(ConstructionParameters const& construction_parameters)
		: recording(construction_parameters.device, construction_parameters.graphics, construction_parameters.global_layout, construction_parameters.object_layout, construction_parameters.allocator)
	{}

	void ViewContext::PrepareThreads(ViewParameters const& view_parameters) {
		//Prepare culling parameters
		culling.frustum = glm::identity<glm::mat4>();
		while (culling.thread_mesh_collections.size() < view_parameters.num_culling_threads) {
			culling.thread_mesh_collections.emplace_back();
		}
		culling.thread_workers.reserve(view_parameters.num_culling_threads);

		//Prepare recording parameters
		recording.command_pool.Reset();
		while (recording.thread_command_buffers.size() < view_parameters.num_recording_threads) {
			recording.thread_command_buffers.emplace_back(recording.command_pool.CreateBuffer(ECommandBufferLevel::Secondary));
		}
		recording.thread_workers.reserve(view_parameters.num_recording_threads);
	}

	void ViewContext::Clear(ResourcesCollection& previous_resources) {
		for (ThreadMeshCollection& mesh_collection : culling.thread_mesh_collections) {
			for (StaticMeshParameters& static_mesh : mesh_collection.static_meshes) {
				previous_resources << static_mesh.pipeline_resources;
				previous_resources << static_mesh.mesh_resources;
			}
			mesh_collection.static_meshes.clear();
		}
	}

	FrameContext::FrameContext(VkDevice device, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator)
		: command_pool(device, graphics)
		, image_available_semaphore(device, 0)
		, fence(device)
		, view_construction_parameters(device, graphics, global_layout, object_layout, allocator)
	{}

	void FrameContext::Prepare(std::span<ViewParameters const> views_parameters, ResourcesCollection& previous_resources) {
		command_pool.Reset();

		//Clear all existing views to wipe the resources they are holding onto.
		for (ViewContext& view : views) {
			view.Clear(previous_resources);
		}

		//Resize the number of views, down to at least one
		size_t const num_views = std::max<size_t>(views_parameters.size(), 1);
		if (views.size() < num_views) {
			size_t const num_added = num_views - views.size();
			for (size_t index = 0; index < num_added; ++index) {
				views.emplace_back(view_construction_parameters);
				view_command_buffers.emplace_back(command_pool.CreateBuffer(ECommandBufferLevel::Primary));
			}
		}
		else if (views.size() > num_views) {
			size_t const num_removed = views.size() - num_views;
			for (size_t index = 0; index < num_removed; ++index) {
				command_pool.DestroyBuffer(view_command_buffers.back());

				views.pop_back();
				view_command_buffers.pop_back();
			}
		}

		//Prepare the per-thread resources that each view will need
		for (size_t view_index = 0; view_index < num_views; ++view_index) {
			views[view_index].PrepareThreads(views_parameters[view_index]);
		}
	}
}
