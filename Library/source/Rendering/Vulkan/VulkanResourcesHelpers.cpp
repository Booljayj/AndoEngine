#include "Engine/LinearStrings.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanResourcesHelpers.h"

#include <ios>


namespace Rendering {
	VulkanShaderModuleLibrary::VulkanShaderModuleLibrary(VkDevice inDevice)
	: device(inDevice)
	{}

	VulkanShaderModuleLibrary::~VulkanShaderModuleLibrary() {
		for (Entry entry : entries) {
			if (entry.module) vkDestroyShaderModule(device, entry.module, nullptr);
		}
	}

	VkShaderModule VulkanShaderModuleLibrary::GetModule(CTX_ARG, std::string_view name) {
		Hash32 const nameHash = Hash32{ name };

		//Try to find an existing loaded shader entry
		auto const iter = std::find_if(entries.begin(), entries.end(), [=](auto const& entry) { return entry.hash == nameHash; });
		if (iter != entries.end()) return iter->module;

		//Create a new loaded shader entry and return that
		entries.push_back(Entry{});
		Entry& newEntry = entries[entries.size() - 1];
		newEntry.hash = nameHash;

		TEMP_ALLOCATOR_MARK();

		std::string_view const filename = l_printf(CTX.temp, "content/shaders/compiled/%s.spv", name.data());
		std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			LOGF(Vulkan, Error, "Failed to open file %s for shader %s", filename.data(), name.data());
			return nullptr;
		}

		size_t const bufferSize = (size_t) file.tellg();
		char* const buffer = static_cast<char*>(CTX.temp.Request(bufferSize, sizeof(char), alignof(uint32_t)));

		file.seekg(0);
		file.read(buffer, bufferSize);
		file.close();

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = bufferSize;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer);

		VkShaderModule module = nullptr;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Failed to create shader module for shader %s", name.data());
			return nullptr;
		}

		newEntry.module = module;
		return module;
	}

	void VulkanMeshCreationHelper::Submit(VulkanMeshCreationResults const& result) {
		//Keep track of the results
		results.push_back(result);

		//Submit the transfer requests
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &result.commands;

		vkQueueSubmit(logical->queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);

		//Increment the number of iterations. If we've uploaded enough data, flush the queue so we can remove temporary resources
		++flushIterations;
		if (flushIterations > 1023) {
			Flush();
		}
	}

	void VulkanMeshCreationHelper::Flush() {
		flushIterations = 0;
		vkQueueWaitIdle(logical->queues.graphics);
		vkResetCommandPool(logical->device, pool, 0);
		for (VulkanMeshCreationResults& result : results) {
			result.staging.vertex.Destroy(logical->allocator);
			result.staging.index.Destroy(logical->allocator);
		}
		results.clear();
	}
}
