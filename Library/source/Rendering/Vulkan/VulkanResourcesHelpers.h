#pragma once
#include "Engine/Hash.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Resources/Resource.h"

namespace Rendering {
	struct Shader;

	/** A library of loaded shader modules which can be re-used to create pipelines */
	struct VulkanPipelineCreationHelper {
	public:
		VulkanPipelineCreationHelper(VkDevice inDevice);
		~VulkanPipelineCreationHelper();

		/** Get an already-loaded module, or load a new one */
		VkShaderModule GetModule(Resources::Handle<Shader> shader);

	private:
		struct Entry {
			Resources::Identifier id;
			VkShaderModule module = nullptr;
		};

		VkDevice device;
		std::vector<Entry> entries;
	};

	struct VulkanMeshCreationHelper {
	public:
		VulkanMeshCreationHelper(VkDevice inDevice, VkQueue inQueue, VkCommandPool inPool);
		VulkanMeshCreationHelper(VulkanMeshCreationHelper const&) = delete;
		VulkanMeshCreationHelper(VulkanMeshCreationHelper&&) = delete;
		~VulkanMeshCreationHelper();

		void Submit(VkCommandBuffer commands, MappedBuffer&& staging);
		void Flush();

	private:
		/** Commands and buffers which haven't been submitted yet */
		std::vector<VkCommandBuffer> pendingCommands;
		std::vector<MappedBuffer> pendingStagingBuffers;
		/** The commands and buffers which were most recently submitted to the queue */
		std::vector<VkCommandBuffer> submittedCommands;
		std::vector<MappedBuffer> submittedStagingBuffers;

		VkDevice device = nullptr;
		VmaAllocator allocator = nullptr;
		VkQueue queue = nullptr;
		VkCommandPool pool = nullptr;

		VkFence fence = nullptr;

		void FinishSubmit();
	};
}
