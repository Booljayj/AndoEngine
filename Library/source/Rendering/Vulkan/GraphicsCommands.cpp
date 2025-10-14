#include "GraphicsCommands.h"
#include "Rendering/Vulkan/GraphicsCommands.h"

namespace Rendering {
	GraphicsCommandWriter::GraphicsCommandWriter(VkCommandBuffer commands)
		: commands(commands)
	{
		VkCommandBufferBeginInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr,
		};

		if (vkBeginCommandBuffer(commands, &info) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to begin recording graphics command buffer" };
		}
	}

	GraphicsCommandWriter::GraphicsCommandWriter(VkCommandBuffer commands, CommandInheritance const& inheritance)
		: commands(commands)
	{
		VkCommandBufferInheritanceInfo inherit{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = inheritance.renderPass,
			.subpass = inheritance.subpass,
			.framebuffer = inheritance.framebuffer,
			.occlusionQueryEnable = inheritance.occlusionQueryEnable,
			.queryFlags = inheritance.queryFlags,
			.pipelineStatistics = inheritance.pipelineStatistics,
		};

		VkCommandBufferBeginInfo const info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &inherit,
		};

		if (vkBeginCommandBuffer(commands, &info) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to begin recording graphics command buffer" };
		}
	}

	GraphicsCommandWriter::~GraphicsCommandWriter() {
		if (vkEndCommandBuffer(commands) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to finish recording command buffer");
		}
	}
}
