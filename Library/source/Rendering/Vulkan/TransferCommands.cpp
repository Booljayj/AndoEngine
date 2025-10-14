#include "Rendering/Vulkan/TransferCommands.h"

Rendering::TransferCommandWriter::TransferCommandWriter(VkCommandBuffer commands)
	: commands(commands)
{
	VkCommandBufferBeginInfo const info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr,
	};

	if (vkBeginCommandBuffer(commands, &info) != VK_SUCCESS) {
		throw std::runtime_error{ "Failed to begin recording transfer command buffer" };
	}
}

Rendering::TransferCommandWriter::~TransferCommandWriter() {
	if (vkEndCommandBuffer(commands) != VK_SUCCESS) {
		LOG(Vulkan, Error, "Failed to finish recording command buffer");
	}
}
