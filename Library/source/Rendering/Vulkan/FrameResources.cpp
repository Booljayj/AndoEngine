#include "Rendering/Vulkan/FrameResources.h"

namespace Rendering {
	FrameUniforms::FrameUniforms(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout global, VkDescriptorSetLayout object, VmaAllocator allocator)
		: sets(device, descriptor_pool, { global, object })
		, global(device, sets[0], 0, allocator)
		, object(device, sets[1], 0, allocator)
	{}

	ThreadResources::ThreadResources(size_t num_objects) {
		constexpr size_t min_num_objects = 512;
		resources.reserve(std::max(num_objects, min_num_objects));
	}

	void ThreadResources::Prepare(size_t num_objects, ResourcesCollection& previous_resources) {
		previous_resources << resources;
		resources.reserve(num_objects);
	}

	constexpr VkDescriptorPoolSize pool_sizes[] = {
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
	};

	ViewResources::ViewResources(VkDevice device, ViewRenderingParameters view_params, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator)
		: command_pool(device, graphics)
		, descriptor_pool(device, pool_sizes, 2)
		, uniforms(device, descriptor_pool, global_layout, object_layout, allocator)
	{
		size_t const num_threads = std::max(view_params.num_threads, 1ull);
		size_t const max_objects_per_thread = (view_params.entities.size() / num_threads) + 1;

		for (size_t index = 0; index < num_threads; ++index) {
			threads.emplace_back(max_objects_per_thread);
			thread_command_buffers.emplace_back(command_pool.CreateBuffer(VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY));
		}
	}

	void ViewResources::Prepare(VkDevice device, ViewRenderingParameters view_params, GraphicsQueue graphics, ResourcesCollection& previous_resources) {
		size_t const num_threads = std::max(view_params.num_threads, 1ull);
		size_t const max_objects_per_thread = (view_params.entities.size() / num_threads) + 1;

		command_pool.Reset();
		uniforms.object.Reserve(view_params.entities.size());

		//All existing threads are prepared, even if they won't be used. This is because we still need to collect previously-used resources, and resetting the command pool is a fairly trivial operation.
		for (auto& thread : threads) {
			thread.Prepare(max_objects_per_thread, previous_resources);
		}
		for (size_t index = threads.size(); index < num_threads; ++index) {
			threads.emplace_back(max_objects_per_thread);
			thread_command_buffers.emplace_back(command_pool.CreateBuffer(VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY));
		}
	}

	FrameResources::FrameResources(VkDevice device, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator)
		: command_pool(device, graphics)
		, image_available_semaphore(device, 0)
		, fence(device)
	{
		views.emplace_back(device, ViewRenderingParameters{}, graphics, global_layout, object_layout, allocator);
		view_command_buffers.emplace_back(command_pool.CreateBuffer(VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY));
	}

	void FrameResources::Prepare(VkDevice device, std::span<ViewRenderingParameters const> view_params, GraphicsQueue graphics, VkDescriptorSetLayout global_layout, VkDescriptorSetLayout object_layout, VmaAllocator allocator, ResourcesCollection& previous_resources) {
		command_pool.Reset();

		if (view_params.size() == 0) {
			//This is a highly unlikely situation where we haven't been given anything to render. Instead of resetting everything back to zero, just assume we can keep the resources and wait until we do have some views.
			return;
		
		} else if (views.size() == view_params.size()) {
			//We don't need to create or destroy any views, we just need to prepare them to be used.
			for (size_t index = 0; index < views.size(); ++index) {
				views[index].Prepare(device, view_params[index], graphics, previous_resources);
			}

		} else if (views.size() < view_params.size()) {
			//We need to grow the number of views to match what will be used. We'll start by preparing the existing views, then create the new ones.
			for (size_t index = 0; index < views.size(); ++index) {
				views[index].Prepare(device, view_params[index], graphics, previous_resources);
			}
			for (size_t index = views.size(); index < view_params.size(); ++index) {
				views.emplace_back(device, view_params[index], graphics, global_layout, object_layout, allocator);
				view_command_buffers.emplace_back(command_pool.CreateBuffer(VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY));
			}

		} else {
			//We have more views than we need, so we'll discard the unused views and prepare the remaining ones that will be used.
			size_t const num_removed = views.size() - view_params.size();
			for (size_t index = 0; index < num_removed; ++index) {
				command_pool.DestroyBuffer(view_command_buffers.back());
				
				views.pop_back();
				view_command_buffers.pop_back();
			}

			for (size_t index = 0; index < views.size(); ++index) {
				views[index].Prepare(device, view_params[index], graphics, previous_resources);
			}
		}
	}
}
