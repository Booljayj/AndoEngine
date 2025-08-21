#pragma once
#include "Engine/Array.h"
#include "Engine/Hash.h"
#include "Engine/StringID.h"
#include "Rendering/Vulkan/Fence.h"
#include "Rendering/Vulkan/Queues.h"
#include "Rendering/Vulkan/Resources.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Resources/Resource.h"

namespace Rendering {
	struct Shader;

	/** A library of loaded shader modules which can be re-used to create pipelines */
	struct PipelineCreationHelper {
	public:
		PipelineCreationHelper(VkDevice device);
		PipelineCreationHelper(PipelineCreationHelper const&) = delete;
		PipelineCreationHelper(PipelineCreationHelper&&) = delete;
		~PipelineCreationHelper();

		/** Get an already-loaded module, or load a new one */
		VkShaderModule GetModule(Resources::Handle<Shader> shader);

	private:
		struct Entry {
			StringID id;
			VkShaderModule module = nullptr;

			Entry(StringID id) : id(id) {}
		};

		VkDevice device = nullptr;
		std::vector<Entry> entries;
	};

	struct MeshCreationHelper {
	public:
		MeshCreationHelper(VkDevice device, Queue transfer, VkCommandPool pool);
		MeshCreationHelper(MeshCreationHelper const&) = delete;
		MeshCreationHelper(MeshCreationHelper&&) = delete;
		~MeshCreationHelper();

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
		Queue transfer;
		VkCommandPool pool = nullptr;

		Fence fence;
		
		void FinishSubmit();
	};
}
