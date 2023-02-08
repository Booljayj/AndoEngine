#include "Engine/Logging.h"
#include "Engine/Temporary.h"
#include "Rendering/Shaders.h"
#include "Rendering/Vulkan/VulkanResourcesHelpers.h"

namespace Rendering {
	VulkanPipelineCreationHelper::VulkanPipelineCreationHelper(VkDevice inDevice)
	: device(inDevice)
	{}

	VulkanPipelineCreationHelper::~VulkanPipelineCreationHelper() {
		for (Entry entry : entries) {
			if (entry.module) vkDestroyShaderModule(device, entry.module, nullptr);
		}
	}

	VkShaderModule VulkanPipelineCreationHelper::GetModule(Resources::Handle<Shader> shader) {
		Resources::Identifier const id = shader->id;

		//Try to find an existing loaded shader entry
		auto const iter = std::find_if(entries.begin(), entries.end(), [=](auto const& entry) { return entry.id == id; });
		if (iter != entries.end()) return iter->module;

		//No existing entry was found, so make a new one. Even if we fail to actually create this shader, this entry should always be returned
		auto& newEntry = entries.emplace_back();
		newEntry.id = id;

		if (shader->bytecode.size() > 0) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = shader->bytecode.size() * sizeof(decltype(shader->bytecode)::value_type);
			createInfo.pCode = shader->bytecode.data();

			VkShaderModule module = nullptr;
			if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create shader module for shader %i", shader->id.ToValue());
				return nullptr;
			}

			newEntry.module = module;
			return module;
		}
		return nullptr;
	}

	void VulkanMeshCreationHelper::Submit(VulkanMappedBuffer const& staging, VkCommandBuffer commands) {
		constexpr size_t NumStagingBuffersPerFlush = 512;
		//Keep track of the results
		stagingBuffers.push_back(staging);

		if (commands) {
			//Submit the transfer requests
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commands;
			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		}

		//If we've uploaded enough data, flush the queue so we can remove temporary resources
		if (stagingBuffers.size() >= NumStagingBuffersPerFlush) {
			Flush();
		}
	}

	void VulkanMeshCreationHelper::Flush() {
		vkQueueWaitIdle(queue);
		vkResetCommandPool(device, pool, 0);
		for (VulkanMappedBuffer const& staging : stagingBuffers) {
			staging.Destroy(allocator);
		}
		stagingBuffers.clear();
	}
}
