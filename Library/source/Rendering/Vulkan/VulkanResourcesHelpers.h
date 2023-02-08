#pragma once
#include "Engine/Hash.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
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
		std::vector<VulkanMappedBuffer> stagingBuffers;

		VkDevice device = nullptr;
		VmaAllocator allocator = nullptr;
		VkQueue queue = nullptr;
		VkCommandPool pool = nullptr;

		VulkanMeshCreationHelper(VkDevice inDevice, VmaAllocator inAllocator, VkQueue inQueue, VkCommandPool inPool)
		: device(inDevice), allocator(inAllocator), queue(inQueue), pool(inPool)
		{}

		void Submit(VulkanMappedBuffer const& staging, VkCommandBuffer commands);
		void Flush();
	};
}
