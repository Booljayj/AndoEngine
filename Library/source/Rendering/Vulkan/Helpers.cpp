#include "Engine/Logging.h"
#include "Engine/Ranges.h"
#include "Rendering/Shader.h"
#include "Rendering/Vulkan/Helpers.h"

namespace Rendering {
	PipelineCreationHelper::PipelineCreationHelper(VkDevice device)
		: device(device)
	{}

	PipelineCreationHelper::~PipelineCreationHelper() {
		for (Entry entry : entries) {
			if (entry.module) vkDestroyShaderModule(device, entry.module, nullptr);
		}
	}

	VkShaderModule PipelineCreationHelper::GetModule(Resources::Handle<Shader> shader) {
		//Try to find an existing loaded shader entry
		auto const iter = ranges::find_if(entries, [&](auto const& entry) { return entry.id == shader->GetName(); });
		if (iter != entries.end()) return iter->module;

		//No existing entry was found, so make a new one. Even if we fail to actually create this shader, this entry should always be returned for this id.
		auto& entry = entries.emplace_back(shader->GetName());

		if (shader->bytecode.size() > 0) {
			VkShaderModuleCreateInfo const moduleCI{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = shader->bytecode.size() * sizeof(decltype(shader->bytecode)::value_type),
				.pCode = shader->bytecode.data(),
			};
			
			if (vkCreateShaderModule(device, &moduleCI, nullptr, &entry.module) != VK_SUCCESS || !entry.module) {
				LOG(Vulkan, Error, "Failed to create shader module for shader {}", shader->GetName());
			}
		}

		return entry.module;
	}

	MeshCreationHelper::MeshCreationHelper(VkDevice device, Queue transfer, VkCommandPool pool)
		: device(device), transfer(transfer), pool(pool), fence(device)
	{}

	MeshCreationHelper::~MeshCreationHelper() {
		if (pendingCommands.size() > 0) Flush();
		FinishSubmit();
	}

	void MeshCreationHelper::Submit(VkCommandBuffer commands, MappedBuffer&& staging) {
		constexpr size_t NumStagingBuffersPerFlush = 512;

		//Keep track of the results
		pendingCommands.emplace_back(commands);
		pendingStagingBuffers.emplace_back(std::move(staging));

		//If we've collected enough data, flush the queue to send the commands to the GPU
		if (pendingStagingBuffers.size() >= NumStagingBuffersPerFlush) Flush();
	}

	void MeshCreationHelper::Flush() {
		//Finish the previous submit
		FinishSubmit();

		//Prepare for the new submit
		fence.Reset();
		std::swap(pendingCommands, submittedCommands);
		std::swap(pendingStagingBuffers, submittedStagingBuffers);

		//Submit the current commands
		VkSubmitInfo const submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = static_cast<uint32_t>(submittedCommands.size()),
			.pCommandBuffers = submittedCommands.data(),
		};
		transfer.Submit(submitInfo, fence);
	}

	void MeshCreationHelper::FinishSubmit() {
		//Wait for the previous flush to finish
		fence.WaitUntilSignalled(std::chrono::nanoseconds::max());
		
		//Clean up the temporary resources from the previous flush
		if (submittedCommands.size() > 0) {
			vkFreeCommandBuffers(device, pool, static_cast<uint32_t>(submittedCommands.size()), submittedCommands.data());
			submittedCommands.clear();
			submittedStagingBuffers.clear();
		}
	}
}
