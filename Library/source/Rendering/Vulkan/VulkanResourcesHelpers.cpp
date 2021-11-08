#include "Engine/LogCommands.h"
#include "Engine/Temporary.h"
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

	VkShaderModule VulkanPipelineCreationHelper::GetModule(std::string_view name) {
		Hash32 const nameHash = Hash32{name};

		//Try to find an existing loaded shader entry
		auto const iter = std::find_if(entries.begin(), entries.end(), [=](auto const& entry) { return entry.hash == nameHash; });
		if (iter != entries.end()) return iter->module;

		//Create a new loaded shader entry and return that
		entries.push_back(Entry{});
		Entry& newEntry = entries[entries.size() - 1];
		newEntry.hash = nameHash;

		SCOPED_TEMPORARIES();

		std::string_view const filename = t_printf("content/shaders/compiled/%s.spv", name.data());
		std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			LOGF(Vulkan, Error, "Failed to open file %s for shader %s", filename.data(), name.data());
			return nullptr;
		}

		size_t const bufferSize = static_cast<size_t>(file.tellg());
		t_vector<char> buffer;
		buffer.resize(bufferSize);

		file.seekg(0);
		file.read(buffer.data(), bufferSize);
		file.close();

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = bufferSize;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

		VkShaderModule module = nullptr;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Failed to create shader module for shader %s", name.data());
			return nullptr;
		}

		newEntry.module = module;
		return module;
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
